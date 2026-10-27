#pragma once

#include <stddef.h>
#include <stdint.h>
typedef enum {
  TOKEN_IDENTIFIER,
  TOKEN_INTEGER,
  TOKEN_COLON,
  TOKEN_COMMA,
  TOKEN_DIRECTIVE,
  TOKEN_NEWLINE,
  TOKEN_EOF
} TokenType;

typedef struct {
  TokenType type;
  const char *start;
  size_t length;
  size_t line;
  size_t column;
} Token;

typedef struct {
    const char *source;
    size_t start;
    size_t current;
    size_t line;
    size_t column;
    size_t start_column;
} Lexer;

void lexer_init(Lexer *lexer, const char *source);
char lexer_peek(const Lexer *lexer);
char lexer_advance(Lexer *lexer);

Token lexer_next(Lexer *lexer);
