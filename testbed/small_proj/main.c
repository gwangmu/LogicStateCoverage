#include <stdio.h>
#include <string.h>

int main(int argc, char **argv) {
  if (argc == 1) {
    printf("yeah");
    if (!strcmp(argv[0], "a")) 
      printf("what");
    else
      printf("whatup");
  }
  return 0;
}
