#include <stdio.h>
#include <assert.h>
#include "malloc_common.h"

void test_basic_alloc_free(void) {
    int* ptr = (int*)my_malloc(sizeof(int));
    printf("%p\n", ptr);
    assert(ptr != NULL);
    *ptr = 42;
    my_free(ptr);
}

void test_multiple_allocs(void) {
    int* ptrs[100];
    for (int i = 0; i < 100; i++) {
        ptrs[i] = (int*)my_malloc(sizeof(int));
        assert(ptrs[i] != NULL);
        *ptrs[i] = i;
    }
    
    // Verify values and free
    for (int i = 0; i < 100; i++) {
        assert(*ptrs[i] == i);
        my_free(ptrs[i]);
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
