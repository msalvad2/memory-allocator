#include "allocator.h" // assert(condition)
#include <stdint.h> // uintptr_t
#include <assert.h> // assert(condition)
#include <stdio.h> //printf
#include <unistd.h> //sbrk
#include <string.h>

int main(int argc, char * argv[]){
    printf("Test: \n");

    void* heap_start = sbrk(0);
    void* heap_end = sbrk(0);
    // Testing allocator reuses freed memory instead of calling Operating system
    printf("---Reusability Test---\n");
    char * a = malloc( 2 * sizeof(char));
    free(a); //free it
    void* break_before = sbrk(0); // check address
    char * b = malloc(2 * sizeof(char));

    void * break_after = sbrk(0);
    free(b);

    assert(break_before == break_after);
    //should show two blocks - 1024 (printf call) and 16 (reused block)
    heap_dump();

    // testing coalescing 
    printf("---Coalescing Test---\n");
    a = malloc(2 * sizeof(char)); //reuses the memory in heap
    b = malloc(2 * sizeof(char)); // increases program break using sbrk

    heap_start = sbrk(0);
    free(a);
    free(b);
    // should show two blocks = 1024(printf) and 64(merged block)
    heap_dump();

    // splitting test
    printf("---Splitting Test---\n");
    int* a_b = malloc(1 * sizeof(int));
    heap_end = sbrk(0);
    // should show 3 blocks = 1024(printf), 16(used), 16(free)
    heap_dump();
    assert( heap_start == heap_end);
    free(a_b);

    // verifying mmap call works
    printf("---mmap Test---\n");

    
    void * big_alloc = malloc(200*1024); //should use mmap instead of sbrk
    //should display two block - 1024(printf) - 64(merged). mmap block exists outside of heap
    heap_dump();
    assert(big_alloc != NULL);
    memset(big_alloc, 0xAB, 200*1024);
    free(big_alloc);

    // Realloc test
    printf("---Realloc Test---\n");
    // case 1:
    void* pointer = NULL;
    pointer = realloc(pointer, 5);
    assert(pointer != NULL);
    heap_dump();

    // case 2:
    pointer = realloc(pointer, 0);
    assert(pointer == NULL);
    heap_dump();

    // case 3: shrinking
    pointer = malloc(256);
    heap_dump();
    void* smaller_pointer = realloc(pointer, 64);
    // Since it got smaller, you should not move addresses
    heap_dump();
    assert(pointer == smaller_pointer);
    
    // case 4: growing in place by merging next block
    // if you merge blocks you will have just enough space 
    void * larger_pointer = realloc(smaller_pointer, 256);
    heap_dump();

    assert(larger_pointer == smaller_pointer); //address is the same

    // case 5: growing
    // Will have to move to new block in order to allocate memory
    void * largest = realloc(larger_pointer, 512);
    //should have [1024 used ] [352 free] [512 used]
    heap_dump();
    header_t* largest_header = (header_t*)largest - 1;
    size_t size = largest_header->size;
    assert(size == 512);
    free(largest);




    int length = 10;
    int *num_array = malloc(length * sizeof(int));

    //writing to the memory
    for (int i = 0; i < length; ++i){
        num_array[i] = i * 10;
    }
    for (int i = 0; i < length; ++ i){
        assert(num_array[i] == i*10);
    }

    int *num = malloc(16 * sizeof(int));
    double * num_2 = malloc(17 * sizeof(int));
    char * word = malloc(31 * sizeof(char));

    //Verify it gave intervals of 16
    assert((uintptr_t)num % 16 == 0);
    assert((uintptr_t) num_2 % 16 == 0);
    assert((uintptr_t) word % 16 == 0);

    //verify no overlap
    int size1 = 2;
    int size2 = 2;

    int *num1 = malloc(size1 * sizeof(int));
    int *num2 = malloc(size2 * sizeof(int));


    for (int i = 0; i < 2; ++i){
        num1[i] = (i + 1) * 10;
        num2[i] = i + 1;
    }

    for (int i = 0; i < 2; ++ i){
        assert(num1[i] == (i +1) * 10);
        assert(num2[i] == i + 1);
    }

    
    free(num_array);
    free(num);
    free(num_2);
    free(word);
    free(num1);
    free(num2);
    
    printf("All tests passed\n");
    
    return 0;
}