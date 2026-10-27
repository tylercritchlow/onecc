#include "lexer.h"

void lexer_init(Lexer *lexer, const char *source) {
  lexer->source = source;
  lexer->start = 0;
  lexer->current = 0;
  lexer->line = 1;
  lexer->column = 1;
  lexer->start_column = 1;
}

char lexer_peek(const Lexer *lexer) {
  return lexer->source[lexer->current];
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
