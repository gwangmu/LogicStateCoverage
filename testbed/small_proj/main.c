#include <stdio.h>

int main(int argc, char **argv) {
  if (argc == 1) {
    printf("yeah");
    if (argv[0] == "a") 
      printf("what");
    else
      printf("whatup");
  }
  return 0;
}
