#include "allocator.h"
#include <unistd.h> //sbrk
#include <stdio.h> //perror

static void *heap_start = NULL;


void *malloc(size_t size) {
    //nothing to allocate
    if (size == 0){
        return NULL;
    }
    // set heap_start once per program lifetime
    if (!heap_start) {
        heap_start = sbrk(0);
    }
    size_t aligned_size = ALIGN(size);
    // will see if heap currently has enough memory
    header_t * header = find_free_block(aligned_size);

    if (header) {
        // if memory is big enough will split it to preserve memory
        // You get closest aligned size to memory requested
        header = split_block(header, aligned_size);
    }

    // means could not find memory in current heap
    if (!header){

        size_t total = ALIGN(aligned_size + HEADER_SIZE + FOOTER_SIZE);
        header = sbrk(total);
        // call failed
        if (header == (void*)-1){
            //perror("sbrk"); - allocaters typically stay silent
            return NULL;
       }
        // stores aligned size so heap traversal lands on next header header
        header->size = total - HEADER_SIZE - FOOTER_SIZE; // gives user data size
        // takes you to footer
        footer_t * footer = (footer_t*) ((char*)header + HEADER_SIZE + header->size);
        footer->size = total - HEADER_SIZE - FOOTER_SIZE; //gives user data size

}

    header->free = 0; //used
    //skips header_t because user only wants access to user data
    return (void*)(header + 1);
}

void *realloc(void * ptr, size_t size){
    (void)ptr;
    (void)size;
    return NULL;
}
// releases the memory so it can be reused
void free(void * ptr){

    if (!ptr) return; // user passed in NULL

    // ptr points to the user data
    header_t* header = (header_t*)ptr - 1;
    header->free = 1;

    // find next_header block & see if it is free
    header_t* next_header = (header_t*) ((char*) header + HEADER_SIZE + header->size + FOOTER_SIZE);
    // if next_header block exists and is free
    if ((void*)next_header < sbrk(0) && next_header->free == 1){
        // absorb next_header into header using header->size
        header->size = header->size + FOOTER_SIZE  + HEADER_SIZE + next_header->size;
        // update footer->size to reflect change
        footer_t* footer = (footer_t*) ((char*)next_header + HEADER_SIZE + next_header->size);
        footer->size = header->size; 
    }

    if ((void*) header > heap_start){
    // find the prev_header block & see if it is free
    footer_t* prev_footer = (footer_t*) ((char*) header - FOOTER_SIZE);
    
    header_t * prev_header = (header_t*) ((char*) header - FOOTER_SIZE - prev_footer->size - HEADER_SIZE);
    if (prev_header->free == 1){
    prev_header->size = prev_header->size + FOOTER_SIZE + HEADER_SIZE + header->size;
    
    footer_t* footer = (footer_t*) ((char*) header + HEADER_SIZE + header->size);
    footer->size = prev_header->size;
    }
    }

}
// finds the next available block of memory
void* find_free_block (size_t requested){
    
    if ( heap_start == NULL) return NULL;

    header_t* current = heap_start;

    //traversin the heap, sbrk(0) -returns the program break
    while ((void*)current < sbrk(0)){
        // if memory is free
        if (current-> free == 1){
            // if there is enough memory
            if (current->size >= requested){
                return current;
            }
        }
        // heap traversal - use char* cast to move correct number of bytes
        current = (header_t*) ((char*)current + HEADER_SIZE + current->size + FOOTER_SIZE );
    }

    return NULL;

}

void* split_block(header_t* ptr, size_t requested){
    // not worth splitting
    if ( (ptr->size - requested ) < MIN_BLOCK_SIZE){
        return ptr;
    }
    size_t total_bytes = HEADER_SIZE + ptr->size + FOOTER_SIZE;
    // BEFORE SPLITTING:[header | free memory | footer]
    // AFTER SPLITTING: [header | used memory | footer] [new header | new free memory | new footer]
    size_t new_free_bytes = total_bytes - 2*HEADER_SIZE -2*FOOTER_SIZE - requested;
    header_t* header = ptr;
    footer_t* footer = (footer_t*) ((char*)header + HEADER_SIZE + requested);

    header_t* new_header = (header_t*) (footer + 1);
    footer_t* new_footer = (footer_t*) ((char*) new_header + HEADER_SIZE + new_free_bytes);

    header->size = requested;
    header->free = 0;
    footer->size = requested;

    new_header->size = new_free_bytes;
    new_header->free = 1;
    new_footer->size = new_free_bytes;

    return header;


}
// printf allocates a 1024-byte I/O buffer through our malloc on first call
void heap_dump(){

    if (!heap_start){
        printf("[ Heap is empty]\n");
        return;
    }
    header_t* current = heap_start;
    int block_count = 0;

    // sbrk(0) - program break
    while ((void*)current < sbrk(0)){
        char* mode = NULL;
        if ( current->free == 1){
            mode = "FREE";
        }
        else{
            mode = "USED";
        }
        printf("[Block %d | Address: %p | Size %4zu | %s  ]\n", block_count, (void*)current, current->size, mode);
        block_count++;
        current = (header_t*) ((char*)current + HEADER_SIZE + current->size + FOOTER_SIZE);
    }
    printf("----------------\n");
}