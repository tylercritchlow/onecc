#include "assembler/parser.h"
#include "greatest.h"
#include <stdlib.h>
#include <string.h>

GREATEST_MAIN_DEFS();

TEST init_loads_first_token(void) {
  Lexer lexer;
  Parser parser;
  lexer_init(&lexer, "  label:");
  parser_init(&parser, &lexer);

  ASSERT_EQ(TOKEN_IDENTIFIER, parser.current.type);
  ASSERT_MEM_EQ("label", parser.current.start, parser.current.length);
  ASSERT_EQ(5, parser.current.length);
  ASSERT_EQ(1, parser.current.line);
  ASSERT_EQ(3, parser.current.column);
  ASSERT_EQ(TOKEN_EOF, parser.previous.type);
  ASSERT_EQ(0, parser.previous.length);
  ASSERT_FALSE(parser.had_error);
  PASS();
}

TEST advance_preserves_previous_token(void) {
  Lexer lexer;
  Parser parser;
  lexer_init(&lexer, "label:\n");
  parser_init(&parser, &lexer);

  parser_advance(&parser);
  ASSERT_EQ(TOKEN_IDENTIFIER, parser.previous.type);
  ASSERT_EQ(TOKEN_COLON, parser.current.type);
  parser_advance(&parser);
  ASSERT_EQ(TOKEN_COLON, parser.previous.type);
  ASSERT_EQ(TOKEN_NEWLINE, parser.current.type);
  parser_advance(&parser);
  ASSERT_EQ(TOKEN_NEWLINE, parser.previous.type);
  ASSERT_EQ(TOKEN_EOF, parser.current.type);
  ASSERT_EQ(2, parser.current.line);
  ASSERT_EQ(1, parser.current.column);
  parser_advance(&parser);
  ASSERT_EQ(TOKEN_EOF, parser.previous.type);
  ASSERT_EQ(TOKEN_EOF, parser.current.type);
  ASSERT_FALSE(parser.had_error);
  PASS();
}

TEST empty_source_starts_at_eof(void) {
  Lexer lexer;
  Parser parser;
  lexer_init(&lexer, "");
  parser_init(&parser, &lexer);

  ASSERT_EQ(TOKEN_EOF, parser.current.type);
  ASSERT_EQ(1, parser.current.line);
  ASSERT_EQ(1, parser.current.column);
  ASSERT_FALSE(parser.had_error);
  PASS();
}

TEST errors_remain_visible_until_advanced(void) {
  Lexer lexer;
  Parser parser;
  lexer_init(&lexer, "@ label @");
  parser_init(&parser, &lexer);

  ASSERT_EQ(TOKEN_ERROR, parser.current.type);
  ASSERT(parser.had_error);
  parser_advance(&parser);
  ASSERT_EQ(TOKEN_ERROR, parser.previous.type);
  ASSERT_EQ(TOKEN_IDENTIFIER, parser.current.type);
  ASSERT(parser.had_error);
  parser_advance(&parser);
  ASSERT_EQ(TOKEN_ERROR, parser.current.type);
  parser_advance(&parser);
  ASSERT_EQ(TOKEN_EOF, parser.current.type);
  ASSERT(parser.had_error);

  lexer_init(&lexer, "valid");
  parser_init(&parser, &lexer);
  ASSERT_FALSE(parser.had_error);
  ASSERT_EQ(TOKEN_EOF, parser.previous.type);
  PASS();
}

TEST check_does_not_consume(void) {
  Lexer lexer;
  Parser parser;
  lexer_init(&lexer, "label:");
  parser_init(&parser, &lexer);

  ASSERT(parser_check(&parser, TOKEN_IDENTIFIER));
  ASSERT_FALSE(parser_check(&parser, TOKEN_COLON));
  ASSERT_EQ(TOKEN_IDENTIFIER, parser.current.type);
  ASSERT_EQ(TOKEN_EOF, parser.previous.type);
  ASSERT_EQ(5, lexer.current);
  ASSERT_FALSE(parser.had_error);
  PASS();
}

TEST match_consumes_only_on_success(void) {
  Lexer lexer;
  Parser parser;
  lexer_init(&lexer, "label:");
  parser_init(&parser, &lexer);

  ASSERT_FALSE(parser_match(&parser, TOKEN_COMMA));
  ASSERT_EQ(TOKEN_IDENTIFIER, parser.current.type);
  ASSERT_EQ(TOKEN_EOF, parser.previous.type);
  ASSERT_EQ(5, lexer.current);
  ASSERT_FALSE(parser.had_error);
  ASSERT(parser_match(&parser, TOKEN_IDENTIFIER));
  ASSERT_EQ(TOKEN_IDENTIFIER, parser.previous.type);
  ASSERT_EQ(TOKEN_COLON, parser.current.type);
  ASSERT(parser_match(&parser, TOKEN_COLON));
  ASSERT(parser_match(&parser, TOKEN_EOF));
  ASSERT(parser_check(&parser, TOKEN_EOF));
  PASS();
}

TEST expect_consumes_on_success(void) {
  Lexer lexer;
  Parser parser;
  lexer_init(&lexer, "label:");
  parser_init(&parser, &lexer);

  ASSERT(parser_expect(&parser, TOKEN_IDENTIFIER));
  ASSERT_EQ(TOKEN_IDENTIFIER, parser.previous.type);
  ASSERT_EQ(TOKEN_COLON, parser.current.type);
  ASSERT(parser_expect(&parser, TOKEN_COLON));
  ASSERT(parser_expect(&parser, TOKEN_EOF));
  ASSERT_FALSE(parser.had_error);
  PASS();
}

TEST expect_failure_preserves_token_for_recovery(void) {
  Lexer lexer;
  Parser parser;
  lexer_init(&lexer, "\n  label:");
  parser_init(&parser, &lexer);
  parser_advance(&parser);

  ASSERT_FALSE(parser_expect(&parser, TOKEN_COMMA));
  ASSERT(parser.had_error);
  ASSERT_EQ(TOKEN_IDENTIFIER, parser.current.type);
  ASSERT_EQ(2, parser.current.line);
  ASSERT_EQ(3, parser.current.column);
  ASSERT_EQ(TOKEN_NEWLINE, parser.previous.type);
  ASSERT_EQ(8, lexer.current);
  ASSERT(parser_expect(&parser, TOKEN_IDENTIFIER));
  ASSERT(parser.had_error);
  PASS();
}

TEST expect_reports_missing_token_at_eof(void) {
  Lexer lexer;
  Parser parser;
  lexer_init(&lexer, "");
  parser_init(&parser, &lexer);

  ASSERT_FALSE(parser_expect(&parser, TOKEN_INTEGER));
  ASSERT(parser.had_error);
  ASSERT(parser_check(&parser, TOKEN_EOF));
  ASSERT_EQ(0, lexer.current);
  PASS();
}

TEST parse_integer_operands(void) {
  const struct {
    const char *source;
    int64_t value;
  } cases[] = {
      {"0,", 0},
      {"42,", 42},
      {"-1,", -1},
      {"00012,", 12},
      {"-0,", 0},
      {"9223372036854775807,", INT64_MAX},
      {"-9223372036854775808,", INT64_MIN},
  };

  for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); ++i) {
    Lexer lexer;
    Parser parser;
    Operand operand;
    lexer_init(&lexer, cases[i].source);
    parser_init(&parser, &lexer);

    ASSERT(parser_parse_operand(&parser, &operand));
    ASSERT_EQ(OPERAND_INTEGER, operand.type);
    ASSERT_EQ(cases[i].value, operand.value.integer);
    ASSERT_EQ(TOKEN_INTEGER, parser.previous.type);
    ASSERT_EQ(TOKEN_COMMA, parser.current.type);
    ASSERT_FALSE(parser.had_error);
  }
  PASS();
}

TEST parse_symbol_operand(void) {
  const char *source = "\n  counter_2,";
  Lexer lexer;
  Parser parser;
  Operand operand;
  lexer_init(&lexer, source);
  parser_init(&parser, &lexer);
  parser_advance(&parser);

  ASSERT(parser_parse_operand(&parser, &operand));
  ASSERT_EQ(OPERAND_SYMBOL, operand.type);
  ASSERT(operand.value.symbol.start == source + 3);
  ASSERT_EQ(9, operand.value.symbol.length);
  ASSERT_EQ(2, operand.value.symbol.line);
  ASSERT_EQ(3, operand.value.symbol.column);
  ASSERT_EQ(TOKEN_IDENTIFIER, operand.value.symbol.type);
  ASSERT_EQ(TOKEN_COMMA, parser.current.type);
  ASSERT_FALSE(parser.had_error);
  PASS();
}

TEST reject_invalid_operands(void) {
  const char *sources[] = {
      "9223372036854775808",
      "-9223372036854775809",
      "999999999999999999999999999999999999999",
      ",",
      ":",
      ".word",
      "\n",
      "",
      "@",
  };
  for (size_t i = 0; i < sizeof(sources) / sizeof(sources[0]); ++i) {
    Lexer lexer;
    Parser parser;
    Operand operand = {.type = OPERAND_INTEGER, .value.integer = 123};
    lexer_init(&lexer, sources[i]);
    parser_init(&parser, &lexer);
    Token current = parser.current;
    size_t position = lexer.current;

    ASSERT_FALSE(parser_parse_operand(&parser, &operand));
    ASSERT(parser.had_error);
    ASSERT_EQ(OPERAND_INTEGER, operand.type);
    ASSERT_EQ(123, operand.value.integer);
    ASSERT(parser.current.start == current.start);
    ASSERT_EQ(current.type, parser.current.type);
    ASSERT_EQ(position, lexer.current);
    ASSERT_EQ(TOKEN_EOF, parser.previous.type);
  }
  PASS();
}

TEST integer_conversion_must_match_token_length(void) {
  Lexer lexer;
  Parser parser;
  Operand operand;
  operand.type = OPERAND_INTEGER;
  operand.value.integer = 123;

  lexer_init(&lexer, "42abc");
  parser_init(&parser, &lexer);
  parser.current.length = 5;
  ASSERT_FALSE(parser_parse_operand(&parser, &operand));
  ASSERT(parser.had_error);
  ASSERT_EQ(123, operand.value.integer);
  ASSERT_EQ(2, lexer.current);

  lexer_init(&lexer, "42,");
  parser_init(&parser, &lexer);
  parser.current.length = 1;
  ASSERT_FALSE(parser_parse_operand(&parser, &operand));
  ASSERT(parser.had_error);
  ASSERT_EQ(123, operand.value.integer);
  ASSERT_EQ(2, lexer.current);
  PASS();
}

TEST parse_numeric_instruction(void) {
  Lexer lexer;
  Parser parser;
  Statement statement;
  lexer_init(&lexer, "subleq 9, 10, -1");
  parser_init(&parser, &lexer);

  ASSERT(parser_parse_instruction(&parser, &statement));
  ASSERT_EQ(STMT_INSTRUCTION, statement.type);
  ASSERT_EQ(OPERAND_INTEGER, statement.value.instruction.a.type);
  ASSERT_EQ(9, statement.value.instruction.a.value.integer);
  ASSERT_EQ(OPERAND_INTEGER, statement.value.instruction.b.type);
  ASSERT_EQ(10, statement.value.instruction.b.value.integer);
  ASSERT_EQ(OPERAND_INTEGER, statement.value.instruction.c.type);
  ASSERT_EQ(-1, statement.value.instruction.c.value.integer);
  ASSERT_EQ(TOKEN_EOF, parser.current.type);
  ASSERT_FALSE(parser.had_error);
  PASS();
}

TEST parse_symbolic_instruction(void) {
  Lexer lexer;
  Parser parser;
  Statement statement;
  lexer_init(&lexer, "subleq source, destination, done\n");
  parser_init(&parser, &lexer);

  ASSERT(parser_parse_instruction(&parser, &statement));
  Operand operands[] = {statement.value.instruction.a,
                        statement.value.instruction.b,
                        statement.value.instruction.c};
  const char *names[] = {"source", "destination", "done"};
  for (size_t i = 0; i < 3; ++i) {
    ASSERT_EQ(OPERAND_SYMBOL, operands[i].type);
    ASSERT_EQ(strlen(names[i]), operands[i].value.symbol.length);
    ASSERT_MEM_EQ(names[i], operands[i].value.symbol.start,
                  operands[i].value.symbol.length);
  }
  ASSERT_EQ(TOKEN_NEWLINE, parser.current.type);
  ASSERT_FALSE(parser.had_error);
  PASS();
}

TEST parse_mixed_instruction(void) {
  Lexer lexer;
  Parser parser;
  Statement statement;
  lexer_init(&lexer, "subleq source, 10, -1 ; comment\n");
  parser_init(&parser, &lexer);

  ASSERT(parser_parse_instruction(&parser, &statement));
  ASSERT_EQ(OPERAND_SYMBOL, statement.value.instruction.a.type);
  ASSERT_EQ(OPERAND_INTEGER, statement.value.instruction.b.type);
  ASSERT_EQ(10, statement.value.instruction.b.value.integer);
  ASSERT_EQ(OPERAND_INTEGER, statement.value.instruction.c.type);
  ASSERT_EQ(-1, statement.value.instruction.c.value.integer);
  ASSERT_EQ(TOKEN_NEWLINE, parser.current.type);
  PASS();
}

TEST instruction_failure_stops_without_changing_output(void) {
  const struct {
    const char *source;
    TokenType stopped_at;
  } cases[] = {
      {"add a, b, c", TOKEN_IDENTIFIER},
      {"subleqx a, b, c", TOKEN_IDENTIFIER},
      {".word 1", TOKEN_DIRECTIVE},
      {"", TOKEN_EOF},
      {"subleq", TOKEN_EOF},
      {"subleq , b, c", TOKEN_COMMA},
      {"subleq a b, c", TOKEN_IDENTIFIER},
      {"subleq a, , c", TOKEN_COMMA},
      {"subleq a, b c", TOKEN_IDENTIFIER},
      {"subleq a, b,", TOKEN_EOF},
      {"subleq a, b,\nsubleq 1, 2, 3", TOKEN_NEWLINE},
      {"subleq a, b, @", TOKEN_ERROR},
      {"subleq a, b, 9223372036854775808", TOKEN_INTEGER},
  };
  for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); ++i) {
    Lexer lexer;
    Parser parser;
    Statement statement;
    statement.type = STMT_DATA;
    statement.value.data.type = OPERAND_INTEGER;
    statement.value.data.value.integer = 123;
    lexer_init(&lexer, cases[i].source);
    parser_init(&parser, &lexer);

    ASSERT_FALSE(parser_parse_instruction(&parser, &statement));
    ASSERT(parser.had_error);
    ASSERT_EQ(cases[i].stopped_at, parser.current.type);
    ASSERT_EQ(STMT_DATA, statement.type);
    ASSERT_EQ(OPERAND_INTEGER, statement.value.data.type);
    ASSERT_EQ(123, statement.value.data.value.integer);
  }
  PASS();
}

TEST parse_word_directives(void) {
  Lexer lexer;
  Parser parser;
  Statement statement;
  lexer_init(&lexer, ".word -12\n.word counter");
  parser_init(&parser, &lexer);

  ASSERT(parser_parse_data(&parser, &statement));
  ASSERT_EQ(STMT_DATA, statement.type);
  ASSERT_EQ(OPERAND_INTEGER, statement.value.data.type);
  ASSERT_EQ(-12, statement.value.data.value.integer);
  ASSERT_EQ(TOKEN_NEWLINE, parser.current.type);
  parser_advance(&parser);
  ASSERT(parser_parse_data(&parser, &statement));
  ASSERT_EQ(OPERAND_SYMBOL, statement.value.data.type);
  ASSERT_EQ(7, statement.value.data.value.symbol.length);
  ASSERT_MEM_EQ("counter", statement.value.data.value.symbol.start, 7);
  ASSERT_EQ(TOKEN_EOF, parser.current.type);
  PASS();
}

TEST reject_invalid_word_directives(void) {
  const char *sources[] = {".byte 1", ".words 1", "word 1",
                           ".word",   ".word ,",  ".word 9223372036854775808"};
  for (size_t i = 0; i < sizeof(sources) / sizeof(sources[0]); ++i) {
    Lexer lexer;
    Parser parser;
    Statement statement;
    statement.type = STMT_DATA;
    statement.value.data.type = OPERAND_INTEGER;
    statement.value.data.value.integer = 123;
    lexer_init(&lexer, sources[i]);
    parser_init(&parser, &lexer);

    ASSERT_FALSE(parser_parse_data(&parser, &statement));
    ASSERT(parser.had_error);
    ASSERT_EQ(STMT_DATA, statement.type);
    ASSERT_EQ(123, statement.value.data.value.integer);
  }
  PASS();
}

TEST parse_standalone_label(void) {
  Lexer lexer;
  Parser parser;
  Statement statement;
  lexer_init(&lexer, "loop:\n");
  parser_init(&parser, &lexer);

  ASSERT(parser_parse_label(&parser, &statement));
  ASSERT_EQ(STMT_LABEL, statement.type);
  ASSERT_EQ(4, statement.value.label.length);
  ASSERT_MEM_EQ("loop", statement.value.label.start, 4);
  ASSERT_EQ(1, statement.value.label.line);
  ASSERT_EQ(1, statement.value.label.column);
  ASSERT_EQ(TOKEN_NEWLINE, parser.current.type);
  PASS();
}

TEST reject_label_without_colon(void) {
  Lexer lexer;
  Parser parser;
  Statement statement;
  statement.type = STMT_DATA;
  lexer_init(&lexer, "loop\n");
  parser_init(&parser, &lexer);

  ASSERT_FALSE(parser_parse_label(&parser, &statement));
  ASSERT(parser.had_error);
  ASSERT_EQ(TOKEN_NEWLINE, parser.current.type);
  ASSERT_EQ(STMT_DATA, statement.type);
  PASS();
}

TEST parse_complete_program(void) {
  const char *source = "; countdown\n\n"
                       "loop:\n"
                       "  subleq one, counter, loop ; repeat\n"
                       "one: .word 1\n"
                       "counter: .word 5\n"
                       "exit: subleq one, one, -1\n"
                       "target: .word loop";
  Lexer lexer;
  Parser parser;
  Program program;
  lexer_init(&lexer, source);
  parser_init(&parser, &lexer);

  ASSERT(parser_parse(&parser, &program));
  const StatementType types[] = {
      STMT_LABEL, STMT_INSTRUCTION, STMT_LABEL,       STMT_DATA,  STMT_LABEL,
      STMT_DATA,  STMT_LABEL,       STMT_INSTRUCTION, STMT_LABEL, STMT_DATA};
  ASSERT_EQ(sizeof(types) / sizeof(types[0]), program.count);
  for (size_t i = 0; i < program.count; ++i) {
    ASSERT_EQ(types[i], program.statements[i].type);
  }
  ASSERT_MEM_EQ("loop", program.statements[0].value.label.start, 4);
  ASSERT_EQ(OPERAND_SYMBOL, program.statements[1].value.instruction.a.type);
  ASSERT_MEM_EQ(
      "one", program.statements[1].value.instruction.a.value.symbol.start, 3);
  ASSERT_MEM_EQ("counter",
                program.statements[1].value.instruction.b.value.symbol.start,
                7);
  ASSERT_MEM_EQ(
      "loop", program.statements[1].value.instruction.c.value.symbol.start, 4);
  ASSERT_EQ(1, program.statements[3].value.data.value.integer);
  ASSERT_EQ(5, program.statements[5].value.data.value.integer);
  ASSERT_EQ(-1, program.statements[7].value.instruction.c.value.integer);
  ASSERT_EQ(OPERAND_SYMBOL, program.statements[9].value.data.type);
  ASSERT_EQ(TOKEN_EOF, parser.current.type);
  ASSERT_FALSE(parser.had_error);
  free(program.statements);
  PASS();
}

TEST statement_errors_stop_the_program(void) {
  const struct {
    const char *source;
    TokenType stopped_at;
  } cases[] = {
      {".word 1, 2", TOKEN_COMMA},
      {".word 1 2", TOKEN_INTEGER},
      {"subleq a, b, c, d", TOKEN_COMMA},
      {"subleq a, b, c .word 1", TOKEN_DIRECTIVE},
      {"subleq a, b,\n.word 1", TOKEN_NEWLINE},
      {".word 1\n.word\n.word 2", TOKEN_NEWLINE},
      {".word 1\n@", TOKEN_ERROR},
      {"loop: @", TOKEN_ERROR},
      {"42", TOKEN_INTEGER},
      {":", TOKEN_COLON},
  };
  for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); ++i) {
    Lexer lexer;
    Parser parser;
    Program program = {0};
    lexer_init(&lexer, cases[i].source);
    parser_init(&parser, &lexer);

    ASSERT_FALSE(parser_parse(&parser, &program));
    ASSERT(parser.had_error);
    ASSERT_EQ(cases[i].stopped_at, parser.current.type);
    ASSERT_EQ(0, program.count);
    ASSERT(program.statements == NULL);
  }
  PASS();
}

TEST parse_empty_and_larger_programs(void) {
  Lexer lexer;
  Parser parser;
  Program program;
  lexer_init(&lexer, " ; comment\n\n");
  parser_init(&parser, &lexer);
  ASSERT(parser_parse(&parser, &program));
  ASSERT_EQ(0, program.count);
  free(program.statements);

  char source[1024] = "";
  for (size_t i = 0; i < 40; ++i) {
    strcat(source, ".word 1\n");
  }
  lexer_init(&lexer, source);
  parser_init(&parser, &lexer);
  ASSERT(parser_parse(&parser, &program));
  ASSERT_EQ(40, program.count);
  for (size_t i = 0; i < program.count; ++i) {
    ASSERT_EQ(STMT_DATA, program.statements[i].type);
    ASSERT_EQ(1, program.statements[i].value.data.value.integer);
  }
  free(program.statements);
  PASS();
}

SUITE(parser) {
  RUN_TEST(parse_word_directives);
  RUN_TEST(reject_invalid_word_directives);
  RUN_TEST(parse_standalone_label);
  RUN_TEST(reject_label_without_colon);
  RUN_TEST(parse_complete_program);
  RUN_TEST(statement_errors_stop_the_program);
  RUN_TEST(parse_empty_and_larger_programs);
  RUN_TEST(parse_numeric_instruction);
  RUN_TEST(parse_symbolic_instruction);
  RUN_TEST(parse_mixed_instruction);
  RUN_TEST(instruction_failure_stops_without_changing_output);
  RUN_TEST(integer_conversion_must_match_token_length);
  RUN_TEST(parse_integer_operands);
  RUN_TEST(parse_symbol_operand);
  RUN_TEST(reject_invalid_operands);
  RUN_TEST(check_does_not_consume);
  RUN_TEST(match_consumes_only_on_success);
  RUN_TEST(expect_consumes_on_success);
  RUN_TEST(expect_failure_preserves_token_for_recovery);
  RUN_TEST(expect_reports_missing_token_at_eof);
  RUN_TEST(init_loads_first_token);
  RUN_TEST(advance_preserves_previous_token);
  RUN_TEST(empty_source_starts_at_eof);
  RUN_TEST(errors_remain_visible_until_advanced);
}

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(parser);
  GREATEST_MAIN_END();
}
