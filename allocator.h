#ifndef ALLOCATOR_H

#define ALLOCATOR_H
#include <stddef.h>

#define ALIGNMENT 16
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1))
#define HEADER_SIZE (sizeof(header_t))
#define FOOTER_SIZE (sizeof(footer_t))

// metadata stored before user data, header size = 16 bytes (8+4) the compiler adds invisible padding so it is divisible by 8
typedef struct header{
    size_t size;
    int free; // 1 = free, 0 = used
} header_t;

// metada stored after user data for coalescing, foot size = 8 bytes
typedef struct footer{
    size_t size;
} footer_t;

void* malloc(size_t  size);
void* realloc(void * ptr, size_t  size);
void free(void * ptr);
void* find_free_block (size_t requested);



#endif