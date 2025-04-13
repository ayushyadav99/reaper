// absolutely naive implementation - Ayush

#include <sys/mman.h>
#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include<unistd.h>
#include<stdlib.h>

struct block_metadata {
  size_t size;  
};

void* malloc(size_t size) {
  //printf("my malloc called\n");

  size_t total_size = size + sizeof(struct block_metadata);

  //size_t page_size = getpagesize();
  //printf("page size: %lu\n", page_size);

  void* p = mmap(NULL, 
                 total_size,
                 PROT_READ | PROT_WRITE,
                 MAP_PRIVATE | MAP_ANON,
                 -1,
                 0);

  if (p == MAP_FAILED) {
    return NULL;
  }

  struct block_metadata* metadata = (struct block_metadata*)p;
  metadata->size = total_size;

  return (void*)(metadata + 1);
}

void free(void* ptr) {
  if (ptr != NULL) {
    struct block_metadata* metadata = ((struct block_metadata*)ptr) - 1;
    munmap(metadata, metadata->size);
  }
}

