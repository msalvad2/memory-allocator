#ifndef ALLOCATOR_H

#define ALLOCATOR_H
#include <stddef.h>

#define ALIGNMENT 16
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1))
#define HEADER_SIZE (sizeof(header_t))

// metadata stored before user data
typedef struct header{
    size_t size;
    int free; // 1 = free, 0 = used
} header_t;

void* malloc(size_t  size);
void* realloc(void * ptr, size_t  size);
void free(void * ptr);
void* find_free_block (size_t requested);



#endif