/*
 * Name: Palla Sanjana Grace
 * Student ID: 241ADB116
 * Week 5 - Task 1: Read and write text files
 */

#include <stdio.h>

int main(int argc, char* argv[]) {
  const char* filename;
  FILE* file;
  char line[256];
  int lines_read = 0;

  /*
   * Use the filename from the command line if one is provided.
   * Otherwise, use data.txt.
   */
  if (argc > 1) {
    filename = argv[1];
  } else {
    filename = "data.txt";
  }

  printf("Writing lines to %s...\n", filename);

  file = fopen(filename, "w");
  if (file == NULL) {
    perror("Could not open file for writing");
    return 1;
  }

  fprintf(file, "Hello, file I/O in C!\n");
  fprintf(file, "This is another line.\n");

  fclose(file);

  printf("Reading contents:\n");

  file = fopen(filename, "r");
  if (file == NULL) {
    perror("Could not open file for reading");
    return 1;
  }

  while (fgets(line, sizeof(line), file) != NULL) {
    printf("%s", line);
    lines_read++;
  }

  fclose(file);

  printf("Lines read: %d\n", lines_read);

  return 0;
}
