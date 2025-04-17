Details about the repository:

1. All of our incremental malloc implementations are inside /src. 
  1. src/malloc_impl_syscall.c is the first iteration.
  2. src/malloc_with_new_freelist.c is the second iteration
  3. src/per_heap_malloc.c implements our Reaper memory allocator. 

2. The header is present at include/malloc_common.h

3. All experiment test code is present in test/
  1. test/test_correctness.c tests the correctness of a memory allocator
  2. test/test_scalability.c is used for the scalability and lock contention experiments. 
  3. test/test_cache_thrash.c is used for testing cache locality.
  4. test/test_cache_scratch.c is used for testing false sharing.

4. All results and graphics/graphics related code can be found in the /results directory.

Details about running the program:
1. To generate an executable for particular experiment with our Reaper allocator (src/per_thread_heap.c) you want to run the following make command from the root directory of the project:
>> make run file=per_thread_heap_malloc test=<test_name> flags=-DUSE_MY_MALLOC=<1/0>

here <test_name> can be the name of any of test files without the extension. And example command is given below:
>> make run file=per_thread_heap_malloc test=test_scalability flags=-DUSE_MY_MALLOC=1

If the flag USE_MY_MALLOC is set to 1, then the generated executable will use the reaper memory allocator. If the flag USE_MY_MALLOC is set to 0, then libc's malloc will be used.
An example of the flag being set to 0 is also given below:
>> make run file=per_thread_heap_malloc test=test_scalability flags=-DUSE_MY_MALLOC=0

To execute the generated executable, you must provide the required command line arguments which can be found inside the test file that you compiled with. 

- In order to run the benchmarks for the hoard memory allocator, follow the steps given below:
    - Clone the source code for Hoard using the command git clone https://github.com/emeryberger/Hoard
    - Run the make command in the source directory generate the libhoard.so shared object
    - set the LD_PRELOAD flag using export LD_PRELOAD=/path/to/libhoard.so
    - compile the needed experiemnt using "make run file=per_thread_heap_malloc test=test_scalability flags=-DUSE_MY_MALLOC=0"
    - make sure the USE_MY_MALLOC flag is set to 0.
    - run the executable by giving the required command line arguments (information about the command line arguments can be found in the test file)


All other implementation details can be found in the report.
