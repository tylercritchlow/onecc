#include "file.h"

#include <stdio.h>
#include <stdlib.h>

char *read_file(const char *path) {
  FILE *file = fopen(path, "rb");
  if (file == NULL) {
    perror(path);
    return NULL;
  }
  size_t capacity = 4096;
  size_t length = 0;
  char *source = malloc(capacity);
  if (source == NULL) {
    fclose(file);
    fprintf(stderr, "Out of memory\n");
    return NULL;
  }
  int ch;
  while ((ch = fgetc(file)) != EOF) {
    if (length == capacity - 1) {
      if (capacity > (size_t)-1 / 2) {
        fprintf(stderr, "Source too large\n");
        free(source);
        fclose(file);
        return NULL;
      }
      capacity *= 2;
      char *grown = realloc(source, capacity);
      if (grown == NULL) {
        fprintf(stderr, "Out of memory\n");
        free(source);
        fclose(file);
        return NULL;
      }
      source = grown;
    }
    if (ch == 0) {
      fprintf(stderr, "%s: embedded NUL in source\n", path);
      free(source);
      fclose(file);
      return NULL;
    }
    source[length++] = (char)ch;
  }
  if (ferror(file)) {
    perror(path);
    free(source);
    fclose(file);
    return NULL;
  }
  fclose(file);
  source[length] = '\0';

  return source;
}
