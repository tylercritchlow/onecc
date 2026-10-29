#include "assembler/lexer.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "Usage: %s <file>\n", argv[0]);
    return 1;
  }
  FILE *file = fopen(argv[1], "rb");
  if (file == NULL) {
    perror(argv[1]);
    return 1;
  }
  size_t capacity = 4096;
  size_t length = 0;
  char *source = malloc(capacity);
  if (source == NULL) {
    fclose(file);
    fprintf(stderr, "Out of memory\n");
    return 1;
  }
  int ch;
  while ((ch = fgetc(file)) != EOF) {
    if (length == capacity - 1) {
      if (capacity > (size_t)-1 / 2) {
        fprintf(stderr, "Source too large\n");
        free(source);
        fclose(file);
        return 1;
      }
      capacity *= 2;
      char *grown = realloc(source, capacity);
      if (grown == NULL) {
        fprintf(stderr, "Out of memory\n");
        free(source);
        fclose(file);
        return 1;
      }
      source = grown;
    }
    if (ch == 0) {
      fprintf(stderr, "%s: embedded NUL in source\n", argv[1]);
      free(source);
      fclose(file);
      return 1;
    }
    source[length++] = (char)ch;
  }
  if (ferror(file)) {
    perror(argv[1]);
    free(source);
    fclose(file);
    return 1;
  }
  fclose(file);
  source[length] = '\0';

  Lexer lexer;
  lexer_init(&lexer, source);
  int status = 0;
  for (;;) {
    Token token = lexer_next(&lexer);
    if (token.type == TOKEN_ERROR) {
      fprintf(stderr, "%s:%zu:%zu: unexpected character 0x%02x\n", argv[1],
              token.line, token.column, (unsigned char)*token.start);
      status = 1;
    } else {
      printf("%zu:%zu %s", token.line, token.column, token_name(token.type));
      if (token.type != TOKEN_NEWLINE && token.type != TOKEN_EOF) {
        putchar(' ');
        fwrite(token.start, 1, token.length, stdout);
      }
      putchar('\n');
    }
    if (token.type == TOKEN_EOF) {
      break;
    }
  }
  free(source);
  return status;
}
