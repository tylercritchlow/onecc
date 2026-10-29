#include "parser.h"

#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void parser_init(Parser *parser, Lexer *lexer) {
  *parser = (Parser){.lexer = lexer, .current = {.type = TOKEN_EOF}};
  parser_advance(parser);
}

void parser_advance(Parser *parser) {
  parser->previous = parser->current;
  parser->current = lexer_next(parser->lexer);
  if (parser->current.type == TOKEN_ERROR) {
    fprintf(stderr, "%zu:%zu: invalid character 0x%02x\n", parser->current.line,
            parser->current.column, (unsigned char)*parser->current.start);
    parser->had_error = true;
  }
}

bool parser_check(const Parser *parser, TokenType type) {
  return parser->current.type == type;
}

bool parser_match(Parser *parser, TokenType type) {
  if (!parser_check(parser, type)) {
    return false;
  }
  parser_advance(parser);
  return true;
}

bool parser_expect(Parser *parser, TokenType type) {
  if (parser_match(parser, type)) {
    return true;
  }

  fprintf(stderr, "%zu:%zu: syntax error: expected %s, got %s\n",
          parser->current.line, parser->current.column, token_name(type),
          token_name(parser->current.type));
  parser->had_error = true;
  return false;
}

bool parser_parse_operand(Parser *parser, Operand *out) {
  Token token = parser->current;

  if (token.type == TOKEN_IDENTIFIER) {
    out->type = OPERAND_SYMBOL;
    out->value.symbol = token;
    parser_advance(parser);
    return true;
  }

  if (token.type == TOKEN_INTEGER) {
    char *end;
    errno = 0;
    intmax_t number = strtoimax(token.start, &end, 10);

    if (end == token.start || end != token.start + token.length) {
      fprintf(stderr, "%zu:%zu: invalid integer\n", token.line, token.column);
      parser->had_error = true;
      return false;
    }

    if (errno == ERANGE || number < INT64_MIN || number > INT64_MAX) {
      fprintf(stderr, "%zu:%zu: integer out of range for int64_t\n", token.line,
              token.column);
      parser->had_error = true;
      return false;
    }

    out->type = OPERAND_INTEGER;
    out->value.integer = (int64_t)number;
    parser_advance(parser);
    return true;
  }

  fprintf(stderr,
          "%zu:%zu: syntax error: expected integer or identifier, got %s\n",
          token.line, token.column, token_name(token.type));
  parser->had_error = true;
  return false;
}

bool parser_parse_instruction(Parser *parser, Statement *out) {
  Token token = parser->current;
  if (token.type != TOKEN_IDENTIFIER || token.length != 6 ||
      memcmp(token.start, "subleq", 6) != 0) {
    fprintf(stderr, "%zu:%zu: syntax error: expected 'subleq'\n", token.line,
            token.column);
    parser->had_error = true;
    return false;
  }
  parser_advance(parser);

  Operand a;
  Operand b;
  Operand c;
  if (!parser_parse_operand(parser, &a)) {
    return false;
  }
  if (!parser_expect(parser, TOKEN_COMMA)) {
    return false;
  }
  if (!parser_parse_operand(parser, &b)) {
    return false;
  }
  if (!parser_expect(parser, TOKEN_COMMA)) {
    return false;
  }
  if (!parser_parse_operand(parser, &c)) {
    return false;
  }

  out->type = STMT_INSTRUCTION;
  out->value.instruction.a = a;
  out->value.instruction.b = b;
  out->value.instruction.c = c;
  return true;
}

bool parser_parse_data(Parser *parser, Statement *out) {
  Token token = parser->current;
  if (token.type != TOKEN_DIRECTIVE || token.length != 5 ||
      memcmp(token.start, ".word", 5) != 0) {
    fprintf(stderr, "%zu:%zu: syntax error: expected '.word'\n", token.line,
            token.column);
    parser->had_error = true;
    return false;
  }
  parser_advance(parser);

  Operand operand;
  if (!parser_parse_operand(parser, &operand)) {
    return false;
  }
  out->type = STMT_DATA;
  out->value.data = operand;
  return true;
}

bool parser_parse_label(Parser *parser, Statement *out) {
  Token label = parser->current;
  if (!parser_expect(parser, TOKEN_IDENTIFIER)) {
    return false;
  }
  if (!parser_expect(parser, TOKEN_COLON)) {
    return false;
  }
  out->type = STMT_LABEL;
  out->value.label = label;
  return true;
}

bool parser_parse_statement(Parser *parser, Statement *out) {
  while (parser_match(parser, TOKEN_NEWLINE)) {
  }
  if (parser_check(parser, TOKEN_EOF)) {
    return false;
  }

  Statement statement;
  if (parser_check(parser, TOKEN_IDENTIFIER)) {
    /* Look ahead without moving the parser's lexer. */
    Lexer lookahead = *parser->lexer;
    Token next = lexer_next(&lookahead);
    if (next.type == TOKEN_COLON) {
      return parser_parse_label(parser, out);
    }
    if (!parser_parse_instruction(parser, &statement)) {
      return false;
    }
  } else if (parser_check(parser, TOKEN_DIRECTIVE)) {
    if (!parser_parse_data(parser, &statement)) {
      return false;
    }
  } else {
    fprintf(stderr, "%zu:%zu: syntax error: expected statement, got %s\n",
            parser->current.line, parser->current.column,
            token_name(parser->current.type));
    parser->had_error = true;
    return false;
  }

  if (!parser_check(parser, TOKEN_EOF) &&
      !parser_expect(parser, TOKEN_NEWLINE)) {
    return false;
  }
  *out = statement;
  return true;
}

bool parser_parse(Parser *parser, Program *out) {
  Program program = {0};
  size_t capacity = 0;
  Statement statement;

  while (!parser->had_error && parser_parse_statement(parser, &statement)) {
    if (program.count == capacity) {
      size_t next_capacity = capacity == 0 ? 16 : capacity * 2;
      if (next_capacity < capacity ||
          next_capacity > SIZE_MAX / sizeof(Statement)) {
        fprintf(stderr, "Program too large\n");
        parser->had_error = true;
        break;
      }
      Statement *grown =
          realloc(program.statements, next_capacity * sizeof(Statement));
      if (grown == NULL) {
        fprintf(stderr, "Out of memory\n");
        parser->had_error = true;
        break;
      }
      program.statements = grown;
      capacity = next_capacity;
    }
    program.statements[program.count++] = statement;
  }

  if (parser->had_error) {
    free(program.statements);
    return false;
  }
  *out = program;
  return true;
}
