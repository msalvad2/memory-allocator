#include "allocator.h"
#include <unistd.h> //sbrk
#include <stdio.h> //perror

void *malloc(size_t size) {
    //nothing to allocate
    if (size == 0){
        return NULL;
    }
    //cast to header to access header fields
    header_t * block =  sbrk(ALIGN(size + HEADER_SIZE));
    // call failed
    if (block == (void*)-1){
        //perror("sbrk"); - allocaters typically stay silent
        return NULL;
    }
    block->size = size;
    block->free = 0; //used

    return (void*)(block + 1);
}

void *realloc(void * ptr, size_t size){
    (void)ptr;
    (void)size;
    return NULL;
}
void free(void * ptr){
    (void)ptr;
}