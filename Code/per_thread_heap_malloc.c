#include <stdio.h>
#include <omp.h>


void *global_block_pointer = NULL;

int main() {
    #pragma omp parallel
    {
        int id = omp_get_thread_num();
        printf("Hello from thread %d\n", id);
    }

    return 0;
}