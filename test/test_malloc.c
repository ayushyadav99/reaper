#include <stdio.h>
#include <assert.h>

#define USE_MY_MALLOC 1 
#if USE_MY_MALLOC
#include "malloc_common.h"
#define MALLOC my_malloc
#define FREE my_free
#else
#include<stdlib.h>
#define MALLOC malloc
#define FREE free
#endif

void test_basic_alloc_free(void) {
<<<<<<< HEAD
    int* ptr = (int*)my_malloc(sizeof(int));
    printf("%p\n", ptr);
    assert(ptr != NULL);
    *ptr = 42;
    my_free(ptr);
=======
    int* ptr = (int*)MALLOC(sizeof(int));
    printf("%p\n", ptr);
    assert(ptr != NULL);
    *ptr = 42;
    FREE(ptr);
>>>>>>> origin
}

void test_multiple_allocs(void) {
    int* ptrs[100];
    for (int i = 0; i < 100; i++) {
<<<<<<< HEAD
        ptrs[i] = (int*)my_malloc(sizeof(int));
=======
        ptrs[i] = (int*)MALLOC(sizeof(int));
>>>>>>> origin
        assert(ptrs[i] != NULL);
        *ptrs[i] = i;
    }
    
    // Verify values and free
    for (int i = 0; i < 100; i++) {
        assert(*ptrs[i] == i);
<<<<<<< HEAD
        my_free(ptrs[i]);
=======
        FREE(ptrs[i]);
>>>>>>> origin
    }
}

int main(void) {
    printf("Running basic allocation test...\n");
    test_basic_alloc_free();
    
    printf("Running multiple allocations test...\n");
    test_multiple_allocs();
    
    printf("All tests passed!\n");
    return 0;
}
