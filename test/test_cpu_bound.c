#include<omp.h>
#include <stdio.h>
#include <assert.h>


#ifdef USE_MY_MALLOC_ENV
  #include "malloc_common.h"
  #define MALLOC my_malloc
  #define FREE my_free
#else
  #include<stdlib.h>
  #define MALLOC malloc
  #define FREE free
#endif

#define NUM_THREADS 8

int foo(int num)
{
    long long ans = 0.0;
    for(int i = 0; i<1000000; ++i)
    {
        ans = ans + (num*i);
    }

    int ret = (int)(ans);
    return ret;
}

int main()
{
    double tstart = 0.0;
    double tend = 0.0;
    double ttaken = 0.0;

    tstart = omp_get_wtime(); //getting the start time

    int *arr = (int*)MALLOC(100*sizeof(int));

    int a = 1;
    int b = 2;
    int c = a + b;
    int d = a*c + b;
    int sum_i = 0;

    for(int i = 0; i<100; ++i)
    {
        arr[i] = foo((c+d)*i);
        sum_i += arr[i];
    }

    FREE(arr);


    tend = omp_get_wtime(); // getting the end time
    ttaken = tend - tstart; // calculating the time taken

    printf("Time taken: %lf\n", ttaken);
    return 0;

}

