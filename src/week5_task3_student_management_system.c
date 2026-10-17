/*
 * Name: Palla Sanjana Grace
 * Student ID: 241ADB116
 * Week 5 - Task 3: Student management system
 */

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_STUDENTS 100

typedef struct Student {
  char name[50];
  int id;
  float gpa;
} Student;

int read_line(char* buffer, size_t size) {
  size_t length;

  if (fgets(buffer, size, stdin) == NULL) {
    return 0;
  }

  length = strlen(buffer);

  if (length > 0 && buffer[length - 1] == '\n') {
    buffer[length - 1] = '\0';
  } else {
    int character;

    while ((character = getchar()) != '\n' && character != EOF) {
      /* Remove extra characters from the input line. */
    }
  }

  return 1;
}

int parse_int(const char* text, int* value) {
  char* end;
  long result;

  errno = 0;
  result = strtol(text, &end, 10);

  while (isspace((unsigned char)*end)) {
    end++;
  }

  if (text == end || *end != '\0' || errno != 0) {
    return 0;
  }

  *value = (int)result;
  return 1;
}

int parse_float(const char* text, float* value) {
  char* end;
  float result;

  errno = 0;
  result = strtof(text, &end);

  while (isspace((unsigned char)*end)) {
    end++;
  }

  if (text == end || *end != '\0' || errno != 0) {
    return 0;
  }

  *value = result;
  return 1;
}

int load_students(Student arr[], int max) {
  FILE* file;
  char line[256];
  int count = 0;

  file = fopen("students.txt", "r");

  if (file == NULL) {
    return 0;
  }

  while (count < max && fgets(line, sizeof(line), file) != NULL) {
    Student student;
    char extra;

    if (sscanf(line, " %49[^,],%d,%f %c", student.name, &student.id,
               &student.gpa, &extra) == 3) {
      if (student.name[0] != '\0' && student.gpa >= 0.0f &&
          student.gpa <= 4.0f) {
        arr[count] = student;
        count++;
      }
    }
  }

  fclose(file);
  return count;
}

int save_students(const Student arr[], int count) {
  FILE* file;
  int i;

  file = fopen("students.txt", "w");

  if (file == NULL) {
    return -1;
  }

  for (i = 0; i < count; i++) {
    if (fprintf(file, "%s,%d,%.2f\n", arr[i].name, arr[i].id, arr[i].gpa) < 0) {
      fclose(file);
      return -1;
    }
  }

  if (fclose(file) != 0) {
    return -1;
  }

  return 0;
}

void add_student(Student arr[], int* count) {
  char input[128];
  Student student;

  if (*count >= MAX_STUDENTS) {
    printf("Cannot add more students (limit reached).\n");
    return;
  }

  printf("Enter name: ");

  if (!read_line(input, sizeof(input)) || input[0] == '\0' ||
      strchr(input, ',') != NULL || strlen(input) >= sizeof(student.name)) {
    printf("Invalid input. Student not added.\n");
    return;
  }

  strcpy(student.name, input);

  printf("Enter ID: ");

  if (!read_line(input, sizeof(input)) || !parse_int(input, &student.id)) {
    printf("Invalid input. Student not added.\n");
    return;
  }

  printf("Enter GPA: ");

  if (!read_line(input, sizeof(input)) || !parse_float(input, &student.gpa) ||
      student.gpa < 0.0f || student.gpa > 4.0f) {
    printf("Invalid input. Student not added.\n");
    return;
  }

  arr[*count] = student;
  (*count)++;

  printf("Student added successfully!\n");
}

void list_students(const Student arr[], int count) {
  int i;

  if (count == 0) {
    printf("No students found.\n");
    return;
  }

  printf("ID    Name                 GPA\n");

  for (i = 0; i < count; i++) {
    printf("%-5d %-20s %.2f\n", arr[i].id, arr[i].name, arr[i].gpa);
  }
}

int main(void) {
  Student students[MAX_STUDENTS];
  char input[128];
  int count;
  int option;

  count = load_students(students, MAX_STUDENTS);

  printf("Loaded %d student(s) from students.txt\n", count);

  while (1) {
    printf("\n=== Student Management System ===\n");
    printf("1. List students\n");
    printf("2. Add student\n");
    printf("3. Save and Exit\n");
    printf("Select an option: ");

    if (!read_line(input, sizeof(input))) {
      if (save_students(students, count) != 0) {
        printf("Could not save students.\n");
        return 1;
      }

      printf("Data saved. Goodbye!\n");
      return 0;
    }

    if (!parse_int(input, &option)) {
      printf("Invalid option.\n");
    } else if (option == 1) {
      list_students(students, count);
    } else if (option == 2) {
      add_student(students, &count);
    } else if (option == 3) {
      if (save_students(students, count) != 0) {
        printf("Could not save students.\n");
        return 1;
      }

      printf("Data saved. Goodbye!\n");
      return 0;
    } else {
      printf("Invalid option.\n");
    }
  }
}
