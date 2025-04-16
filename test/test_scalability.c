#include <math.h>
#include<omp.h>
#include <stdio.h>
#include <assert.h>
#include<sys/time.h>
#include<stdint.h>
#include<stdlib.h>

#if USE_MY_MALLOC == 1
    #include "malloc_common.h"
    #define MALLOC my_malloc
    #define FREE my_free
#else
  #include<stdlib.h>
  #define MALLOC malloc
  #define FREE free
#endif

static unsigned int thread_count = 1;
static uint64_t iteration_count = 1000000;
static unsigned long size = 512;

#define USECSPERSEC 1000000.0

double *execution_time;
void *run_test();
void *dummy(unsigned);

int main(int argc, char **argv) {
  unsigned int i;
  switch (argc)
    {
    case 4:			/* size, iteration count, and thread count were specified */
      thread_count = atoi (argv[3]);
    case 3:			/* size and iteration count were specified; others default */
      iteration_count = atoll (argv[2]);
    case 2:			/* size was specified; others default */
      size = atoi (argv[1]);
    case 1:			/* use default values */
      break;
    default:
      printf ("Unrecognized arguments.\n");
      exit (1);
    }

  printf("Object size: %ld, Iterations: %ld, Threads: %d, malloc_type: %d\n", size, iteration_count, thread_count, USE_MY_MALLOC);
  execution_time = (double *) MALLOC(sizeof(double)* thread_count);

  omp_set_num_threads(thread_count);

  #pragma omp parallel
  {
  run_test();
  }

  double sum = 0.0;
  double stddev = 0.0;
  double average;

  for (i=0; i<thread_count; i++) {
    sum += execution_time[i];
  }
  average = sum/thread_count;

  for (i=0;i<thread_count; i++) {
    double diff = execution_time[i] - average;
    stddev += diff*diff;
  }
  stddev = sqrt(stddev/((thread_count > 1) ? (thread_count-1) : 1));
  if (thread_count > 1) {
    printf ("Average exec time = %f seconds, standard deviation = %f.\n", average, stddev);
  } else {
    printf ("Average exec time = %f seconds.\n", average);
  }
  exit (0);
}

void *run_test() {
  register unsigned int i;
  register unsigned long request_size = size;
  register uint64_t total_iterations = iteration_count;
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
