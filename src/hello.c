
#include <stdio.h>

int main(int argc, char *argv[]) {
  // This is your first C program my friend
  printf("Hello,Sanjana Grace,HBU?\n");
  printf("Most beautiful moment is yet come.\n", argc - 1);
  for (int i = 1; i < argc; ++i) {
    printf("  arg[%d] = %s\n", i, argv[i]);
  }
  return 0;
}
