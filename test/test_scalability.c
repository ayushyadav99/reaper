#include <math.h>
#include<omp.h>
#include <stdio.h>
#include <assert.h>
#include<sys/time.h>

<<<<<<< HEAD
=======

>>>>>>> origin
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

#define NUM_THREADS 8
#define ITERATION_COUNT 1000
#define size 512
#define USECSPERSEC 1000000.0

double *execution_time;
void *run_test();
void *dummy(unsigned);

int main() {
  unsigned int i;
  printf("Object size: %d, Iterations: %d, Threads: %d\n", size, ITERATION_COUNT, NUM_THREADS);
  execution_time = (double *) MALLOC(sizeof(double)* NUM_THREADS); 

  //TODO: see if we need pthread_barrier
  omp_set_num_threads(NUM_THREADS);
  
  #pragma omp parallel
  {
  run_test();
  }

  double sum = 0.0;
  double stddev = 0.0;
  double average;

  for (i=0; i<NUM_THREADS; i++) {
    sum += execution_time[i]; 
  }
  average = sum/NUM_THREADS;
  
  for (i=0;i<NUM_THREADS; i++) {
    double diff = execution_time[i] - average;
    stddev += diff*diff;
  }
  stddev = sqrt(stddev/((NUM_THREADS > 1) ? (NUM_THREADS-1) : 1));
  if (NUM_THREADS > 1) {
    printf ("Average exec time = %f seconds, standard deviation = %f.\n", average, stddev);
  } else {
    printf ("Average exec time = %f seconds.\n", average);
  }
  exit (0);
}

void *run_test() {
  register unsigned int i;
  register unsigned long request_size = size;
  register uint64_t total_iterations = ITERATION_COUNT;
  register double start_time, end_time;


  //ensure all threads start at the same time 
  #pragma omp barrier
  start_time = omp_get_wtime();
  {
    void **buf = (void **)MALLOC(sizeof(void*)*total_iterations);
    for (i=0; i<total_iterations; i++) {
      buf[i] = MALLOC(request_size);
    }
    for(i=0; i<total_iterations; i++){
      FREE(buf[i]);
    }
    FREE(buf);
  }
  end_time = omp_get_wtime(); 
  double elapsed_time = end_time - start_time;

  #pragma omp barrier

  unsigned int pt = omp_get_thread_num();
  execution_time[pt] = elapsed_time;
  return NULL;
}
