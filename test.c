#include "allocator.h" // assert(condition)
#include <stdint.h> // uintptr_t
#include <assert.h> // assert(condition)
#include <stdio.h> //printf
#include <unistd.h> //sbrk

int main(int argc, char * argv[]){

    // Testing allocator reuses freed memory instead of calling Operating system
    int * a = malloc(sizeof(int));

    free(a); //free it
    void* break_before = sbrk(0); // check address
    int * b = malloc(sizeof(int));

    void * break_after = sbrk(0);
    free(b);

    assert(break_before == break_after);

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