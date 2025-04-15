/**
 * @file cache-scratch.cpp
 *
 * cache-scratch is a benchmark that exercises a heap's cache locality.
 * An allocator that allows multiple threads to re-use the same small
 * object (possibly all in one cache-line) will scale poorly, while
 * an allocator like Hoard will exhibit near-linear scaling.
 *
 * Try the following (on a P-processor machine):
 *
 *  cache-scratch 1 1000 1 1000000
 *  cache-scratch P 1000 1 1000000
 *
 *  cache-scratch-hoard 1 1000 1 1000000
 *  cache-scratch-hoard P 1000 1 1000000
 *
 *  The ideal is a P-fold speedup.
*/

#include<omp.h>
#include <stdio.h>
#include <assert.h>
#include<sys/time.h>

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
#define ITERATION_COUNT 100
#define OBJ_SIZE 1 
#define REPETITIONS 10000 

typedef struct {
  char * object;
  int objSize;
  int iterations;
  int repetitions;
} workerArg;

void worker(workerArg *arg) {
  //free obj we were given
  FREE(arg->object);

  //then, repeatedly malloc a given sized object, repeatedly write on it
  //then free it. 
  register int i;
  register int j;
  register int k;

  for (i =0; i<arg->iterations; i++) {
    char *obj = (char*)MALLOC(arg->objSize);

    for(j = 0; j<arg->repetitions; j++) {
      for (k = 0; k < arg->objSize; k++) {
        //repeatedly write and read into the memory (fast if cache locality is good) 
        obj[k] = (char)k;
        volatile char ch = obj[k];
        ch++;
      }
    }
    FREE(obj);
  }
}

int main() {
  omp_set_num_threads(NUM_THREADS);
  workerArg *args = (workerArg *)MALLOC(NUM_THREADS*sizeof(workerArg));
  char **objs = (char **)MALLOC(NUM_THREADS * sizeof(char *));

  register double start_time, end_time; 
  
  for (int i=0; i<NUM_THREADS; i++) {
    objs[i] = (char*)MALLOC(OBJ_SIZE);
    args[i].object = objs[i];
    args[i].objSize = OBJ_SIZE;
    args[i].repetitions = REPETITIONS/NUM_THREADS;
    args[i].iterations = ITERATION_COUNT;
  }
  
  start_time = omp_get_wtime();
  
  #pragma omp parallel
  {
    int id = omp_get_thread_num();
    worker(&args[id]);
  }

  end_time = omp_get_wtime();

  printf("Time elapsed: %f seconds \n", end_time - start_time);

  FREE(args);
  FREE(objs);
}


