#include <sys/mman.h>
#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include<unistd.h>
#include<stdlib.h>

#define malloc(size) my_malloc(size)
#define free(ptr) my_free(ptr)

struct block_metadata {
  size_t size;  
};

void* my_malloc(size_t size) {
  size_t total_size = size + sizeof(struct block_metadata);

  size_t page_size = getpagesize();
  printf("page size: %lu\n", page_size);

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

void my_free(void* ptr) {
  if (ptr != NULL) {
    struct block_metadata* metadata = ((struct block_metadata*)ptr) - 1;
    munmap(metadata, metadata->size);
  }
}

//TEST CODE BELOW
struct Student {
  int id;
  float gpa;
};

int main(int argc, char** argv) {
  if (argc != 2) {
    printf("specify number of mallocs\n");
    return 1;
  }
  const int num_objects = atoi(argv[1]);

  for (int i = 0; i < num_objects; i++) {
  struct Student* student = (struct Student*)malloc(sizeof(struct Student));

  if (student == NULL) {
    printf("Memory allocation failed!\n");
    return 1;
  }
    student->id = i + 1;
    student->gpa = 3.0;  
  printf("student: ID = %d, GPA = %.2f\n", student->id, student->gpa);
  free(student);
  }


  return 0;
}

