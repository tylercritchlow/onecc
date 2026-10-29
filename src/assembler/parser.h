#pragma once

#include "lexer.h"

#include <stdbool.h>
#include <stdint.h>

typedef enum { OPERAND_INTEGER, OPERAND_SYMBOL } OperandType;

typedef struct {
  OperandType type;
  union {
    int64_t integer;
    Token symbol;
  } value;
} Operand;

typedef enum { STMT_INSTRUCTION, STMT_DATA, STMT_LABEL } StatementType;

typedef struct {
  StatementType type;
  union {
    Token label;
    struct {
      Operand a;
      Operand b;
      Operand c;
    } instruction;
    Operand data;
  } value;
} Statement;

typedef struct {
  Statement *statements;
  size_t count;
} Program;

typedef struct {
  Lexer *lexer;
  Token current;
  Token previous;
  bool had_error;
} Parser;

void parser_init(Parser *parser, Lexer *lexer);
void parser_advance(Parser *parser);

bool parser_check(const Parser *parser, TokenType type);
bool parser_match(Parser *parser, TokenType type);
bool parser_expect(Parser *parser, TokenType type);

bool parser_parse_operand(Parser *parser, Operand *out);
bool parser_parse_instruction(Parser *parser, Statement *out);
bool parser_parse_data(Parser *parser, Statement *out);
bool parser_parse_label(Parser *parser, Statement *out);
bool parser_parse_statement(Parser *parser, Statement *out);

bool parser_parse(Parser *parser, Program *out);
