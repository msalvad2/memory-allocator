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

    // will see if heap currently has enough memory
    header_t * header = find_free_block(size);

    // means could not find memory in current heap
    if (!header){

        size_t total = ALIGN(size + HEADER_SIZE + FOOTER_SIZE);
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