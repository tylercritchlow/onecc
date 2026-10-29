#include "lexer.h"

void lexer_init(Lexer *lexer, const char *source) {
  lexer->source = source;
  lexer->start = 0;
  lexer->current = 0;
  lexer->line = 1;
  lexer->column = 1;
  lexer->start_column = 1;
}

char lexer_peek(const Lexer *lexer) { return lexer->source[lexer->current]; }

char lexer_peek_next(const Lexer *lexer) {
  return lexer_peek(lexer) == '\0' ? '\0' : lexer->source[lexer->current + 1];
}

char lexer_advance(Lexer *lexer) {
  if (lexer_peek(lexer) == '\0') {
    return '\0';
  }

  char consumed = lexer->source[lexer->current];

  lexer->current += 1;
  if (consumed == '\n') {
    lexer->line += 1;
    lexer->column = 1;
  } else {
    lexer->column += 1;
  }

  return consumed;
}

static int is_digit(char c) { return c >= '0' && c <= '9'; }

static int is_identifier_start(char c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

static void scan_identifier(Lexer *lexer) {
  while (is_identifier_start(lexer_peek(lexer)) ||
         is_digit(lexer_peek(lexer))) {
    lexer_advance(lexer);
  }
}

Token lexer_next(Lexer *lexer) {
  for (;;) {
    char c = lexer_peek(lexer);
    if (c == ' ' || c == '\t' || c == '\r') {
      lexer_advance(lexer);
    } else if (c == ';') {
      while (lexer_peek(lexer) != '\n' && lexer_peek(lexer) != '\0') {
        lexer_advance(lexer);
      }
    } else {
      break;
    }
  }

  lexer->start = lexer->current;
  lexer->start_column = lexer->column;
  size_t line = lexer->line;
  char c = lexer_advance(lexer);
  TokenType type = TOKEN_ERROR;
  if (is_identifier_start(c)) {
    scan_identifier(lexer);
    type = TOKEN_IDENTIFIER;
  } else if (is_digit(c) || (c == '-' && is_digit(lexer_peek(lexer)))) {
    while (is_digit(lexer_peek(lexer))) {
      lexer_advance(lexer);
    }
    type = TOKEN_INTEGER;
  } else {
    switch (c) {
    case '.':
      if (is_identifier_start(lexer_peek(lexer))) {
        scan_identifier(lexer);
        type = TOKEN_DIRECTIVE;
      }
      break;
    case ':':
      type = TOKEN_COLON;
      break;
    case ',':
      type = TOKEN_COMMA;
      break;
    case '\n':
      type = TOKEN_NEWLINE;
      break;
    case '\0':
      type = TOKEN_EOF;
      break;
    }
  }
  return (Token){type, lexer->source + lexer->start,
                 lexer->current - lexer->start, line, lexer->start_column};
}
