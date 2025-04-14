// absolutely naive implementation - Ayush

#include <sys/mman.h>
#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include<unistd.h>
#include<stdlib.h>


void* my_malloc(size_t size) {
  //printf("my malloc called\n");

  return malloc(size);
}

void my_free(void* ptr) {
  return free(ptr);
}

