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
    int* ptr = (int*)MALLOC(sizeof(int));
    printf("%p\n", ptr);
    assert(ptr != NULL);
    *ptr = 42;
    FREE(ptr);
}

void test_multiple_allocs(void) {
    int* ptrs[100];
    for (int i = 0; i < 100; i++) {
        ptrs[i] = (int*)MALLOC(sizeof(int));
        assert(ptrs[i] != NULL);
        *ptrs[i] = i;
    }
    
    // Verify values and free
    for (int i = 0; i < 100; i++) {
        assert(*ptrs[i] == i);
        FREE(ptrs[i]);
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
