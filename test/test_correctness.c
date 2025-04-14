//test doing a malloc and free iteratively 

#include <stdio.h>
#include <assert.h>
#include "malloc_common.h"


struct Student {
  int id;
  float gpa;
};

int main() {
  const int num_objects = 1000000; 


  for (int i = 0; i < num_objects; i++) {
  struct Student* student = (struct Student*)malloc(sizeof(struct Student));

  if (student == NULL) {
    printf("Memory allocation failed!\n");
    return 1;
  }
    student->id = i + 1;
    student->gpa = 3.0;  
    if(student->id % 100000 == 0) {
      printf("student: ID = %d, GPA = %.2f\n", student->id, student->gpa);
    }
  free(student);
  }


  return 0;
}

