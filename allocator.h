#ifndef ALLOCATOR_H

#define ALLOCATOR_H
#include <stddef.h>
#include <sys/mman.h>

#define ALIGNMENT 16
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1))
#define HEADER_SIZE (sizeof(header_t)) // 
#define FOOTER_SIZE (sizeof(footer_t))

// block must have atleast 16 bytes in order to split
#define MIN_BLOCK_SIZE (HEADER_SIZE + FOOTER_SIZE + 16)
// size must be at least 128kb to us mmap
#define MMAP_THRESHOLD (128 * 1024)
// Valgrind intercepts and replaces name functions called malloc/free/realloc so 
// I redefined mine
#define malloc my_malloc
#define free my_free
#define realloc my_realloc


// metadata stored before user data, header size = 16 bytes (8+4) the compiler adds invisible padding so it is divisible by 8
typedef struct header{
    size_t size;
    int free; // 1 = free, 0 = used
    int mmapped; // 1 = mmap call, 0 = sbrk call
} header_t, footer_t; // footer_t mirrors header_t layout - only size field is used for footer


void* malloc(size_t  size);
void* realloc(void * ptr, size_t size);
void free(void * ptr);
void* find_free_block (size_t requested);
void* split_block(header_t* ptr, size_t requested);
void heap_dump();



#endif