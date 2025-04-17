//correctness test for malloc

#include <stdio.h>
#include <assert.h>
#include <omp.h>
#include<stdlib.h>

#if USE_MY_MALLOC == 1
#include "malloc_common.h"
#define MALLOC my_malloc
#define FREE my_free
#else
  #define MALLOC malloc
  #define FREE free
#endif

#define ASSERT(expr) \
    if (!(expr)) { \
        printf("\033[0;31mFAILED: Correctness test failed!\033[0m\n"); \
        exit(1); \
    }

#define NUM_THREADS 8
#define NUM_ROWS 10000
#define NUM_COLS 1000

int main() {
  omp_set_num_threads(NUM_THREADS);
  int *matrix[NUM_ROWS];

  #pragma omp parallel for
  for (int i = 0; i < NUM_ROWS; i++) {
    matrix[i] = (int *)MALLOC(sizeof(int)*NUM_COLS);
    assert(matrix[i] != NULL); 

    for (int j = 0; j < NUM_COLS; j++) {
      matrix[i][j] = i+j;
    }
  }
  
  for(int i=0; i<NUM_ROWS; i++) {
    for(int j=0; j<NUM_COLS; j++) {
      ASSERT(matrix[i][j] == i+j);
    }
  }
  
  for (int i=0; i<NUM_ROWS; i++) {
    FREE(matrix[i]);
  }
  printf("\033[0;32mPASS: Correctness test passed!\033[0m\n");
  return 0;
}

