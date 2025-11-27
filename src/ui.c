#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

uint32_t print_ask_list(char title[], const char *options[]) {
  char c;
  uint32_t i, answer;

  printf("%s\n", title);

  for (i = 0; options[i] != NULL; i++) {
    printf("[%u] %s\n", i + 1, options[i]);
  }

  write(STDOUT_FILENO, "> ", sizeof "> ");

  while (!scanf("%u", &answer) || answer > i) {
    printf("Input format is not correct! You need to specify between 1 and "
           "%u\n > ",
           i);
  }
  while ((c = getchar()) != '\n' && c != EOF);

  return answer;
}