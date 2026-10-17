/*
 * Name: Palla Sanjana Grace
 * Student ID: 241ADB116
 * Week 5 - Task 2: Save and load structured records
 */

#include <stdio.h>

typedef struct Student {
  char name[50];
  int age;
  float gpa;
} Student;

int save_student(const Student* s, const char* filename) {
  FILE* file;

  if (s == NULL || filename == NULL) {
    return -1;
  }

  file = fopen(filename, "w");

  if (file == NULL) {
    return -1;
  }

  if (fprintf(file, "%s %d %.2f\n", s->name, s->age, s->gpa) < 0) {
    fclose(file);
    return -1;
  }

  if (fclose(file) != 0) {
    return -1;
  }

  return 0;
}

int load_student(const char* filename, Student* out) {
  FILE* file;
  int fields_read;

  if (filename == NULL || out == NULL) {
    return -1;
  }

  file = fopen(filename, "r");

  if (file == NULL) {
    return -1;
  }

  fields_read = fscanf(file, "%49s %d %f", out->name, &out->age, &out->gpa);

  fclose(file);

  if (fields_read != 3) {
    return -1;
  }

  return 0;
}

int main(void) {
  Student original = {"Alice", 21, 3.75f};
  Student loaded = {0};

  printf("Saving student to file...\n");

  if (save_student(&original, "student.txt") != 0) {
    printf("Could not save student.\n");
    return 1;
  }

  printf("Loading student from file...\n");

  if (load_student("student.txt", &loaded) != 0) {
    printf("Could not load student.\n");
    return 1;
  }

  printf("Loaded student: %s, %d, GPA %.2f\n", loaded.name, loaded.age,
         loaded.gpa);

  return 0;
}
