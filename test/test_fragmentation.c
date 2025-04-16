#include <math.h>
#include<omp.h>
#include <stdio.h>
#include <assert.h>
#include<sys/time.h>
#include<stdint.h>
#include<stdlib.h>

// #include <cstdlib>
#if USE_MY_MALLOC == 1
    #include "malloc_common.h"
    #define MALLOC my_malloc
    #define FREE my_free
#else
  #include<stdlib.h>
  #define MALLOC malloc
  #define FREE free
#endif


static int thread_count = 1;
static int arr_size = 1000;
static int arr_2_size = 1000;

int main(int argc, char** argv)
{   
    switch (argc)
    {
    case 4:
      //do nothing
      thread_count = atoi (argv[1]);
      arr_size = atoll (argv[2]);
      arr_2_size = atoi (argv[3]);
    default:
      // use default values
    }
    
    double tstart = 0.0;
    double tend = 0.0;
    double ttaken = 0.0;

    omp_set_num_threads(thread_count);

    tstart = omp_get_wtime(); //getting the start time

    #pragma omp parallel
    {
        int thread_no = omp_get_thread_num();

        int** arr = (int**)malloc(arr_size*(sizeof(int*)));
        for(int i = 0; i<arr_size; ++i)
        {
            arr[i] = (int*)malloc((i+1)*arr_2_size*(sizeof(int)));
        }

        for(int i = 0; i<arr_size; i+=2)
        {
            free(arr[i]);
        }

        int** arr2 = (int**)malloc(arr_size*(sizeof(int*)));
        for(int i = 0; i<arr_size; ++i)
        {
            arr2[i] = (int*)malloc((i+1)*arr_2_size*(sizeof(int)));
        }

        free(arr);

        // cout << "Thread " << thread_no << " Output: Malloc and Free done\n";
    }

    tend = omp_get_wtime(); // getting the end time
    ttaken = tend - tstart; // calculating the time taken

    //cout << "Time taken for the main part: " << ttaken << "\n";
    printf("Time taken for the main part: %f\n",ttaken);

    // sleep(100);

    return 0;

}