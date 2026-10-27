#include "greatest.h"
#include "assembler/lexer.h"

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

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_TEST(advance_ordinary_character);
  RUN_TEST(advance_multiple_characters);
  RUN_TEST(advance_newline);
  RUN_TEST(advance_end_of_source_preserves_position);
  RUN_TEST(advance_preserves_token_start);
  GREATEST_MAIN_END();
}
