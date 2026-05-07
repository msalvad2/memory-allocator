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
    header_t * block = find_free_block(size);

    // means could not find memory in current heap
    if (!block){
        //cast to header to access header fields
        block = sbrk(ALIGN(size + HEADER_SIZE));
        // call failed
        if (block == (void*)-1){
            //perror("sbrk"); - allocaters typically stay silent
            return NULL;
       }
        // stores aligned size so heap traversal lands on next block header
        block->size = ALIGN(size);

}

    block->free = 0; //used
    //skips header_t because user only wants access to user data
    return (void*)(block + 1);
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

}

void* find_free_block (size_t requested){
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
        current = (header_t*) ((char*)current + HEADER_SIZE + current->size );
    }

    return NULL;

}