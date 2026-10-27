#include <stdio.h>

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Usage: %s <file>\n", argv[0]);
    return 1;
  }

  const char *file_path = argv[1];
  FILE *fptr = fopen(file_path, "r");
  if (fptr == NULL) {
    perror(file_path);
    return 1;
  }

  int ch;
  while ((ch = fgetc(fptr)) != EOF) {
    putchar(ch);
  }

  fclose(fptr);
  return 0;
}
