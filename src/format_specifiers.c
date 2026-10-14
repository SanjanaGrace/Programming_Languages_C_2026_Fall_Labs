
#include <stdio.h>
#include <string.h>

int main(void) {
  int number = 42;
  float decimal = 3.14f;
  char letter = 'A';
  char text[100];

  printf("Integer: %d\n", number);
  printf("Float: %.2f\n", decimal);
  printf("Character: %c\n", letter);

  printf("Enter a line of text: ");

  if (fgets(text, sizeof(text), stdin) != NULL) {
    size_t len = strlen(text);

    if (len > 0 && text[len - 1] == '\n') {
      text[len - 1] = '\0';
    }

    printf("You entered: %s\n", text);
  }

  return 0;
}
