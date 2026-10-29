#include "assembler/lexer.h"
#include "greatest.h"
#include <string.h>

GREATEST_MAIN_DEFS();

TEST advance_ordinary_character(void) {
  Lexer lexer;
  lexer_init(&lexer, "s");

  ASSERT_EQ('s', lexer_advance(&lexer));
  ASSERT_EQ(1, lexer.current);
  ASSERT_EQ(2, lexer.column);
  ASSERT_EQ(1, lexer.line);
  PASS();
}

TEST advance_multiple_characters(void) {
  Lexer lexer;
  lexer_init(&lexer, "abc");

  for (size_t i = 0; i < 3; ++i) {
    ASSERT_EQ("abc"[i], lexer_advance(&lexer));
    ASSERT_EQ(i + 1, lexer.current);
    ASSERT_EQ(i + 2, lexer.column);
    ASSERT_EQ(1, lexer.line);
  }
  PASS();
}

TEST advance_newline(void) {
  Lexer lexer;
  lexer_init(&lexer, "\n");
  lexer.line = 4;
  lexer.column = 7;

  ASSERT_EQ('\n', lexer_advance(&lexer));
  ASSERT_EQ(1, lexer.current);
  ASSERT_EQ(5, lexer.line);
  ASSERT_EQ(1, lexer.column);
  PASS();
}

TEST advance_end_of_source_preserves_position(void) {
  Lexer lexer;
  lexer_init(&lexer, "s\n");
  lexer.current = 2;
  lexer.line = 2;
  lexer.column = 1;

  for (size_t i = 0; i < 2; ++i) {
    ASSERT_EQ('\0', lexer_advance(&lexer));
    ASSERT_EQ(2, lexer.current);
    ASSERT_EQ(2, lexer.line);
    ASSERT_EQ(1, lexer.column);
  }
  PASS();
}

TEST advance_preserves_token_start(void) {
  Lexer lexer;
  lexer_init(&lexer, "abc\n");
  lexer.start = 1;
  lexer.current = 2;
  lexer.column = 3;
  lexer.start_column = 2;

  const char expected[] = {'c', '\n', '\0'};
  for (size_t i = 0; i < sizeof(expected); ++i) {
    ASSERT_EQ(expected[i], lexer_advance(&lexer));
    ASSERT_EQ(1, lexer.start);
    ASSERT_EQ(2, lexer.start_column);
  }
  PASS();
}

typedef struct {
  TokenType type;
  const char *text;
  size_t line;
  size_t column;
} ExpectedToken;

static enum greatest_test_res
assert_tokens(const char *source, const ExpectedToken *expected, size_t count) {
  Lexer lexer;
  lexer_init(&lexer, source);

  for (size_t i = 0; i < count; ++i) {
    Token actual = lexer_next(&lexer);
    char context[128];
    snprintf(context, sizeof(context), "token %zu: expected '%s' at %zu:%zu", i,
             expected[i].text, expected[i].line, expected[i].column);
    ASSERT_EQm(context, expected[i].type, actual.type);
    ASSERT_EQm(context, strlen(expected[i].text), actual.length);
    ASSERT_MEM_EQm(context, expected[i].text, actual.start, actual.length);
    ASSERT_EQm(context, expected[i].line, actual.line);
    ASSERT_EQm(context, expected[i].column, actual.column);
  }
  PASS();
}

TEST token_sequence(void) {
  const char *source = "start:\n"
                       "    subleq a, b, -1\n"
                       " a: .word 3";

  const ExpectedToken expected[] = {
      {TOKEN_IDENTIFIER, "start", 1, 1},
      {TOKEN_COLON, ":", 1, 6},
      {TOKEN_NEWLINE, "\n", 1, 7},
      {TOKEN_IDENTIFIER, "subleq", 2, 5},
      {TOKEN_IDENTIFIER, "a", 2, 12},
      {TOKEN_COMMA, ",", 2, 13},
      {TOKEN_IDENTIFIER, "b", 2, 15},
      {TOKEN_COMMA, ",", 2, 16},
      {TOKEN_INTEGER, "-1", 2, 18},
      {TOKEN_NEWLINE, "\n", 2, 20},
      {TOKEN_IDENTIFIER, "a", 3, 2},
      {TOKEN_COLON, ":", 3, 3},
      {TOKEN_DIRECTIVE, ".word", 3, 5},
      {TOKEN_INTEGER, "3", 3, 11},
      {TOKEN_EOF, "", 3, 12},
  };

  CHECK_CALL(
      assert_tokens(source, expected, sizeof(expected) / sizeof(expected[0])));
  PASS();
}

TEST whitespace_and_comments(void) {
  const char *source = " \t; comment\r\n_abc123 0 ; end";
  const ExpectedToken expected[] = {
      {TOKEN_NEWLINE, "\n", 1, 13},
      {TOKEN_IDENTIFIER, "_abc123", 2, 1},
      {TOKEN_INTEGER, "0", 2, 9},
      {TOKEN_EOF, "", 2, 16},
  };

  CHECK_CALL(
      assert_tokens(source, expected, sizeof(expected) / sizeof(expected[0])));
  PASS();
}

TEST invalid_characters_allow_recovery(void) {
  const ExpectedToken expected[] = {
      {TOKEN_ERROR, "@", 1, 1}, {TOKEN_ERROR, "-", 1, 2},
      {TOKEN_ERROR, ".", 1, 3}, {TOKEN_IDENTIFIER, "next", 1, 5},
      {TOKEN_EOF, "", 1, 9},    {TOKEN_EOF, "", 1, 9},
  };

  CHECK_CALL(assert_tokens("@-. next", expected,
                           sizeof(expected) / sizeof(expected[0])));
  PASS();
}

TEST peek_next_and_empty_source(void) {
  Lexer lexer;
  lexer_init(&lexer, "x");
  ASSERT_EQ('\0', lexer_peek_next(&lexer));
  lexer_advance(&lexer);
  ASSERT_EQ('\0', lexer_peek_next(&lexer));
  lexer_init(&lexer, "");
  for (int i = 0; i < 2; ++i) {
    Token token = lexer_next(&lexer);
    ASSERT_EQ(TOKEN_EOF, token.type);
    ASSERT_EQ(0, token.length);
    ASSERT_EQ(1, token.line);
    ASSERT_EQ(1, token.column);
  }
  lexer_init(&lexer, "ab");
  ASSERT_EQ('b', lexer_peek_next(&lexer));
  PASS();
}

SUITE(lexer) {
  RUN_TEST(token_sequence);
  RUN_TEST(whitespace_and_comments);
  RUN_TEST(invalid_characters_allow_recovery);
  RUN_TEST(peek_next_and_empty_source);
  RUN_TEST(advance_ordinary_character);
  RUN_TEST(advance_multiple_characters);
  RUN_TEST(advance_newline);
  RUN_TEST(advance_end_of_source_preserves_position);
  RUN_TEST(advance_preserves_token_start);
}

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(lexer);
  GREATEST_MAIN_END();
}
