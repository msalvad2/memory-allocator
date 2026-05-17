#include "allocator.h" // assert(condition)
#include <stdint.h> // uintptr_t
#include <assert.h> // assert(condition)
#include <stdio.h> //printf
#include <unistd.h> //sbrk
#include <string.h>

int counter = 0;
int fail = 0;
int pass = 0;

#define CHECK(condtion, message)                       \
    if (condtion){                                     \
        ++counter;                                     \
        ++pass;                                        \
        printf("%2d. [PASS]: %s\n", counter, message); \
    }                                                  \
    else{                                              \
        ++counter;                                     \
        ++fail;                                        \
        printf("%2d. [FAIL]: %s\n", counter, message); \
    }

// Unit Test Functions
void test_malloc_size_zero_returns_null(){
    heap_reset();
    void * break_before = sbrk(0);
    void * result = malloc(0);
    void * break_after = sbrk(0);

    CHECK(break_before == break_after, "Malloc: allocating zero bytes does not call sbrk");
    CHECK(result == NULL, "Malloc: allocating zero bytes returns NULL");
}

void test_malloc_size_max_returns_null() {
    heap_reset();
    void * break_before = sbrk(0);
    void * result = malloc(SIZE_MAX);
    void * break_after = sbrk(0);

    CHECK(break_before == break_after, "Malloc: allocating max size does not call sbrk");
    CHECK(result == NULL, "Malloc: allocating size max returns NULL");
}
void test_free_null_pointer_is_valid(){
    heap_reset();
    void * break_before = sbrk(0);
    free(NULL); 
    void * break_after = sbrk(0);
    CHECK(break_before == break_after, "Free: passing NULL does not modify the heap");

}
void test_malloc_pointer_alignment_interval_16(){
    heap_reset();
    void * ptr1 = malloc(1); // smallest possible
    void * ptr2 = malloc(7); // odd number
    void * ptr3 = malloc(15); // one before align
    void * ptr4 = malloc(16); // exact align
    void * ptr5 = malloc(17); // one past align
    void * ptr6 = malloc(MMAP_THRESHOLD + 1); // mmap call

    CHECK(ptr1 && ptr2 && ptr3 && ptr4 && ptr5 && ptr6, "Alignment: all allocations must succeed to test for alignment");
    if (!ptr1 || !ptr2 || !ptr3 || !ptr4 || !ptr5 || !ptr6){
        free(ptr1);
        free(ptr2);
        free(ptr3);
        free(ptr4);
        free(ptr5);
        free(ptr6);
        return;
    }
    // convert addresses to ints, then verify they are divisible by 16
    CHECK(((uintptr_t)ptr1 % ALIGNMENT == 0), "Alignment: smallest possible size aligned correctly");
    CHECK(((uintptr_t)ptr2 % ALIGNMENT == 0), "Alignment: odd request aligned correctly");
    CHECK(((uintptr_t)ptr3 % ALIGNMENT == 0), "Alignment: request one byte smaller than alignment aligned correctly");
    CHECK(((uintptr_t)ptr4 % ALIGNMENT == 0), "Alignment: exact alignment request correctly aligned");
    CHECK(((uintptr_t)ptr5 % ALIGNMENT == 0), "Alignment: request one byte past alignment, aligned correctly");
    CHECK(((uintptr_t)ptr6 % ALIGNMENT == 0), "Alignment: request one byte past mmap call");
    free(ptr1);
    free(ptr2);
    free(ptr3);
    free(ptr4);
    free(ptr5);
    free(ptr6);

}
void test_reuse_freed_block_no_sbrk(){

    heap_reset();
    // REUSABILITY TEST - resusing block of memory without sbrk call
    void * initial = malloc(2 * sizeof(char));
    // stores the address as an int
    uintptr_t initial_addr = (uintptr_t)initial;
    void* heap_before = sbrk(0);
    free(initial); 
    void * reused = malloc(2 * sizeof(char)); 
    void* heap_after = sbrk(0);
    *(char*)reused = 0xAB; // proves pointer is accesible - first byte - segfaults otherwise
    *((char*)reused + (2 * sizeof(char)) - 1) = 0xAB; //proves pointer is accesible - last byte -segfaults otherwise

    CHECK(heap_before == heap_after, "Reuse: freed block is reused without sbrk call");
    CHECK(initial_addr == (uintptr_t)reused, "Reuse: reused pointer matches the freed block address");
    free (reused);
}

void test_coalescing_forward_merged_blocks_no_sbrk(){
    heap_reset();
    // COALESCING TEST- adjacent freed blocks should merge - forward merge
    void* first = malloc(16);
    void* second = malloc(16);
    uintptr_t first_addr = (uintptr_t)first;
    void* break_before = sbrk(0);
    // forward merge after both free
    free(second);
    free(first);
    void* merged = malloc(32); //should not call sbrk but reuse memory 
    CHECK(merged != NULL, "Coalesce (forward): malloc should find merged block");
    if(!merged) return;
    
    void* break_after = sbrk(0);

    CHECK(break_before == break_after, "Coalesce (forward): adjacent freed blocks should merge - forward merge");
    CHECK(first_addr == (uintptr_t)merged, "Coalesce (forward): reused address of first freed block");

    *((char*)merged) = 0xAB; // write to first byte - no segfaults
    *((char*)merged + 31) = 0xAB; // write to last byte - no segfaults

    header_t* header = (header_t*)merged - 1;
    size_t expected = 32 + HEADER_SIZE + FOOTER_SIZE;
    CHECK(header->size == expected, "Coalesce (forward): merged block has correct size");

    free(merged);
}

void test_coalescing_backward_merged_blocks_no_sbrk(){
    heap_reset();
    // COALESCING TEST- adjacent freed blocks should merge - backward merge
    void* first = malloc(16);
    void* second = malloc(16);
    void* break_before = sbrk(0);
    uintptr_t first_addr = (uintptr_t)first;
    // should merge the blocks
    free(first);
    free(second);
    void* merged = malloc(32); //should not call sbrk but reuse memory 
    CHECK(merged != NULL, "Coalesce (backwards):  malloc should find merged block");
    if(!merged) return;
    void* break_after = sbrk(0);

    CHECK(break_before == break_after, "Coalesce (backwards): adjacent freed blocks should merge - backward merge");
    CHECK(first_addr == (uintptr_t)merged, "Coalesce (backwards): reused address of first freed block");
    
    *((char*)merged) = 0xAB; // prove first byte is writable
    *((char*)merged + 31) = 0xAB; // prove last byte is writable

    CHECK(*(char*)merged == (char)0xAB, "Coalesce (backwards): first byte readable after merge");
    CHECK(*((char*)merged + 31) == (char)0xAB, "Coalesce (backwards): last byte readable after merge");

    size_t expected = 32 + HEADER_SIZE + FOOTER_SIZE;
    header_t* header = (header_t*) merged - 1;
    CHECK(header->size == expected, "Coalesce (backwards): merged block has correct size");
    
    free(merged);
}
void test_three_way_coalescing_merges_into_one(){
    heap_reset();
    void* first = malloc(16);
    uintptr_t first_addr = (uintptr_t)first;

    void* middle = malloc(16);
    void* last = malloc(16);
    free(first);
    free(last);
    // three way merge to one block - size 112
    free(middle);

    void* break_before = sbrk(0);
    size_t expected = 3 * ALIGN(16) + 2 * HEADER_SIZE + 2 * FOOTER_SIZE;
    void* merged = malloc(expected);

    CHECK(merged != NULL, "Coalesce (three way merge): merged block should be reusable");
    if (!merged) return;

    void* break_after = sbrk(0);

    header_t* header = (header_t*)merged - 1;
    CHECK(header->size == expected, "Coalesce (three way): merged block has correct size");
    // write to first and last byte
    *(char*)merged = 0xAB;
    *((char*)merged + expected - 1) = 0xAB;

    CHECK(break_before == break_after, "Coalesce (three way merge): reuses merged block without modifying heap" );
    CHECK(first_addr == (uintptr_t)merged, "Coalesce (three way merge): merged block start at first block's address");
    free(merged);
    
}
void test_split_large_block_leaves_free_remainder(){
    heap_reset();
    //SPLIT TEST: free memory should be split if big enough
    void* original =  malloc(124); // returns in intervals of 16
    void* break_before = sbrk(0);

    header_t* original_header = (header_t*)original - 1;
    size_t requested = ALIGN(64);
    size_t expected = original_header->size - requested - HEADER_SIZE - FOOTER_SIZE;
    free(original);

    // should reuse block and split free excess
    void* split_pointer = malloc(64);
    CHECK(split_pointer != NULL, "Splitting: malloc should not return NULL");
    if (!split_pointer) return;
    void* break_after = sbrk(0);

    header_t* header = (header_t*)split_pointer - 1;
    header_t* next_header = (header_t*)((char*)header + HEADER_SIZE + header->size + FOOTER_SIZE);
    // writing to first and last byte — segfaults if memory is inaccessible,
    // valgrind reports invalid write if boundary is rong
    *(char*)split_pointer = 0xAB;
    *((char*)split_pointer + 63) = 0xAB;

    CHECK(header->size == requested , "Splitting: correct bytes are kept");
    CHECK(next_header->size == expected, "Splitting: correct bytes are freed");
    CHECK(next_header->free == 1, "Splitting: remaining free memory is split to a free block");
    CHECK(break_before == break_after, "Splitting: no sbrk call used");
    free(split_pointer);

}

void test_mmap_free_calls_munmap_(){
    heap_reset();
    // this test will be verified by valgrind, if munmap was not called correctly
    // valgrind will report it as a leak. No need for check
    void* large = malloc(MMAP_THRESHOLD);
    free(large);

}

void test_large_allocation_uses_mmap_not_sbrk(){
    heap_reset();
    // MMAP TEST: large allocations should not happen in the heap
    void * heap_before = sbrk(0);
    void * large = malloc(MMAP_THRESHOLD);
    CHECK(large != NULL, "mmap allocation: allocation returns valid pointer");
    if(large == NULL) return;

    void * heap_after = sbrk(0);

    header_t* header = (header_t*)large - 1;
    CHECK(header->mmapped == 1, "mmap allocation: mmapped flag set in header");
    
    *(char*)large = 0xAB;
    *((char*)large + (MMAP_THRESHOLD) - 1) = 0xAB;

    CHECK(heap_before == heap_after, "mmap allocation: large allocations use mmap instead of sbrk");
    free(large);
}


void test_realloc_null_ptr_acts_as_malloc(){
    heap_reset();
    // REALLOC - Passing null should malloc the size
    void* result = realloc(NULL, 16);
    CHECK(result != NULL, "Realloc: passing a null pointer returns non-null pointer");
    if(result == NULL) return;
    *(char*)result = 0xAB; //write to first byte
    *((char*)result + 15) = 0xAB; //write to last byte

    header_t* header = (header_t*)result - 1;
    
    CHECK(header->size == ALIGN(16), "Realloc: passing a null pointer acts as malloc");

    free(result);
}
/////
void test_realloc_size_zero_acts_as_free(){
    heap_reset();
    // REALLOC - passing size zero should free the pointer
    void * initial = malloc(64);

    // This test will be verified by valgrind, if realloc did not free
    // pointer a correctly then valgrind will report it as a leak
    void * result = realloc(initial, 0);
    CHECK(result == NULL, "Realloc: size zero returns NULL");

}
void test_realloc_size_zero_and_null_ptr_returns_null(){
    heap_reset();
    void* break_before = sbrk(0);
    void* result = realloc(NULL, 0);
    CHECK(result == NULL, "Realloc: passing null pointer and size zero return null pointer");
    void* break_after = sbrk(0);
    CHECK(break_before == break_after,"Realloc: passing null pointer and size zero doesn't affect heap");
}

void test_realloc_shrinking_in_place(){
    heap_reset();
    void * original = malloc(64);
    uintptr_t original_addr = (uintptr_t) original;

    void * heap_before = sbrk(0);
    void * shrunk = realloc(original, 16);
    CHECK(shrunk != NULL, "Realloc: shrinking in place should return valid pointer");
    if(!shrunk) return;
    *(char*)shrunk = 0xAB;
    *((char*)shrunk + 15) = 0xAB;

    void * heap_after = sbrk(0);

    // shrunk in place so heap (sbrk(0)) should not grow
    CHECK(heap_before == heap_after, "Realloc: shrinking should not grow heap");
    CHECK(original_addr == (uintptr_t)shrunk, "Realloc: shrinking should not change address");
    
    header_t* header = (header_t*)shrunk - 1;
    CHECK(header->size == ALIGN(16), "Realloc: shrinking preserves data size requested");
    
    free(shrunk);

}

void test_realloc_grow_in_place(){
    heap_reset();
    void * original = malloc(32);
    uintptr_t original_addr = (uintptr_t)original;
    // block used to merge with original
    void * neighbor = malloc(64);
    free(neighbor);
    void * heap_before = sbrk(0);
    void * grown = realloc(original, 128);
    CHECK(grown != NULL, "Realloc: growing in place returns non-null pointer");
    if(!grown) return;

    void * heap_after = sbrk(0);

    CHECK(heap_before == heap_after, "Realloc: growing in place should not grow heap");
    CHECK(original_addr == (uintptr_t)grown, "Realloc: growing in place should not change address");

    header_t* header = (header_t*)grown - 1;
    CHECK(header->size == ALIGN(128), "Realloc: growing in place should preserve data size requested");

    free(grown);
}
// grows heap 
void test_realloc_grow_moves_to_new_block(){
    heap_reset();
    void * original = malloc(16);
    *(char*)original = 0xAB; // write to first byte
    *((char*)original + 15) = 0xAB; // write to last byte
    uintptr_t original_addr = (uintptr_t)original;
    void * grown = realloc(original, 160);
    CHECK(grown != NULL, "Realloc: moving to new block returns non-null pointer");
    if(!grown) return;

    CHECK(original_addr != (uintptr_t)grown, "Realloc: growing that moves block should contain different address");
    CHECK(*(char*)grown == (char)0xAB, "Realloc: growing that moves block preserves first byte");
    CHECK(*((char*)grown + 15) == (char)0xAB, "Realloc: growing that moves block preserves last byte ");
    header_t* header = (header_t*)grown - 1;
    CHECK(header->size == ALIGN(160), "Realloc: growing that moves block should preserve data size requested");
    free(grown);
}
void test_stress_10000_calls(){
    heap_reset();
    slot_t slot[100];
    srand(23); //predictibility
    int corruption = 0; 
    // reset
    for (int i = 0; i < 100; ++i){
        slot[i].ptr = NULL;
        slot[i].size = 0;
    }

    for ( int i = 0; i < 10000; ++i){

        int index = rand() % 100;
        // free if not null
        if (slot[index].ptr) {
            // verify write was not corrupted
            if (*(slot[index].ptr) != (char)0xAB || *(slot[index].ptr + slot[index].size - 1) != (char)0xAB){
            ++corruption;
        }
            free(slot[index].ptr);
            slot[index].ptr = NULL;
        }
        // malloc if null
        else{

            // size allocated: up to 4096
            size_t size = (rand() % 4096) + 1;
            slot[index].ptr = malloc(size);
            if(!slot[index].ptr){
                CHECK(0, "Malloc: returns non-null pointer");
                continue;
            }
            slot[index].size = size;
            // write to first/last byte
            *((char*)slot[index].ptr) =  0xAB;
            *((char*)slot[index].ptr + size - 1) = 0xAB;
        }
    }

    // cleanup
    for (int i = 0; i < 100; ++i){
        if(slot[i].ptr) {
            if (*(slot[i].ptr) != (char)0xAB || *(slot[i].ptr + slot[i].size - 1) != (char)0xAB){
                ++corruption;
            }
            free(slot[i].ptr);
            slot[i].ptr = NULL;
        }
    }
    CHECK(corruption == 0, "Stress test: no corruption across 10,000 operations");
    if(corruption)
        printf("Number of corruptions: %d\n", corruption);

}

int main(void){
    printf("Test: \n");
    test_malloc_size_zero_returns_null();
    test_malloc_size_max_returns_null();
    test_free_null_pointer_is_valid();
    test_malloc_pointer_alignment_interval_16();
    test_reuse_freed_block_no_sbrk();
    test_coalescing_forward_merged_blocks_no_sbrk();
    test_coalescing_backward_merged_blocks_no_sbrk();
    test_three_way_coalescing_merges_into_one();
    test_split_large_block_leaves_free_remainder();
    test_mmap_free_calls_munmap_();
    test_large_allocation_uses_mmap_not_sbrk();
    test_realloc_null_ptr_acts_as_malloc();
    test_realloc_size_zero_acts_as_free();
    test_realloc_size_zero_and_null_ptr_returns_null();
    test_realloc_shrinking_in_place();
    test_realloc_grow_in_place();
    test_realloc_grow_moves_to_new_block();
    test_stress_10000_calls();
    printf("\nHeap Result after tests:\n");
    heap_dump();

    
    return 0;
}