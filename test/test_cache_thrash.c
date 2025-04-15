/**
 * @file  cache-thrash.c
 * @brief cache-thrash is a benchmark that exercises a heap's cache-locality.
 *
 * Try the following (on a P-processor machine):
 *
 *  cache-thrash 1 1000 1 1000000
 *  cache-thrash P 1000 1 1000000
 *
 *  The ideal is a P-fold speedup.
*/

#include <stdio.h>
#include <string.h>
#include <omp.h>  
#include <time.h> 

#define NUM_THREADS 8

// #ifdef USE_MY_MALLOC_ENV
//   #include "malloc_common.h"
//   #define MALLOC my_malloc
//   #define FREE my_free
// #else
//   #include<stdlib.h>
//   #define MALLOC malloc
//   #define FREE free
// #endif

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

/* This struct holds arguments for each thread */
typedef struct {
  int objSize;
  int iterations;
  int repetitions;
} workerArg;

/* Worker function to perform memory operations */
void worker(workerArg* w)
{
  int i, j, k;

  for (i = 0; i < w->iterations; i++) {
    //Object alloc 
    char* obj = (char*)MALLOC(w->objSize);
    if (obj == NULL) {
      fprintf(stderr, "Error: Memory allocation failed\n");
      exit(1);
    }
    
    //write operations on the memory
    for (j = 0; j < w->repetitions; j++) {
      for (k = 0; k < w->objSize; k++) {
        //making ch volatile so as to keep it from getting optimized
        obj[k] = (char)k;
        volatile char ch = obj[k];
        ch++;
        //alternate implementation below
        //volatile double d = 1.0;
        //d = d * d + d * d;
      }
    }

    /* Free the object */
    FREE(obj);
  }
}

int main()
{
  int nthreads = NUM_THREADS;
  int iterations = 1000;
  int objSize = 1;
  int repetitions = 1000000;
  double start_time, end_time;

  omp_set_num_threads(nthreads);

  workerArg w;
  w.objSize = objSize;
  w.repetitions = repetitions / nthreads;
  w.iterations = iterations;

  start_time = omp_get_wtime();

  #pragma omp parallel
  {
    //local worker for each thread
    workerArg my_w = w;

    //uncomment for debugging
    //int thread_id = omp_get_thread_num();

    /* Call the worker function */
    worker(&my_w);
  }

  end_time = omp_get_wtime();

  printf("Time elapsed = %f seconds.\n", end_time - start_time);

  return 0;
}
