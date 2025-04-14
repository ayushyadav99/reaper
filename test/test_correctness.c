//test doing a malloc and free iteratively 

#include <stdio.h>
#include <assert.h>
#include <omp.h>

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
      assert(matrix[i][j] == i+j);
    }
  }
  
  for (int i=0; i<NUM_ROWS; i++) {
    FREE(matrix[i]);
  }
  printf("correctness test passed\n");
  return 0;
}

