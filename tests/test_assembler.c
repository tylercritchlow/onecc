#include "assembler/assembler.h"
#include "assembler/symbols.h"
#include "greatest.h"
#include "vm/vm_subleq.h"

#include <stdlib.h>
#include <string.h>

GREATEST_MAIN_DEFS();

static bool assemble_source(const char *source, MachineCode *code) {
  Lexer lexer;
  Parser parser;
  Program program;
  lexer_init(&lexer, source);
  parser_init(&parser, &lexer);
  if (!parser_parse(&parser, &program)) {
    return false;
  }
  bool success = assemble(&program, code);
  free(program.statements);
  return success;
}

TEST symbol_lookup_compares_text_and_length(void) {
  SymbolTable table = {0};
  Lexer lexer;
  lexer_init(&lexer, "counter count Counter");
  Token counter = lexer_next(&lexer);
  Token count = lexer_next(&lexer);
  Token uppercase = lexer_next(&lexer);
  ASSERT(symbols_add(&table, counter, 0));
  ASSERT(symbols_add(&table, count, 3));
  ASSERT(symbols_add(&table, uppercase, 6));

  lexer_init(&lexer, "counter missing");
  Token lookup = lexer_next(&lexer);
  int64_t address = -1;
  ASSERT(symbols_find(&table, lookup, &address));
  ASSERT_EQ(0, address);
  ASSERT(symbols_find(&table, count, &address));
  ASSERT_EQ(3, address);
  ASSERT(symbols_find(&table, uppercase, &address));
  ASSERT_EQ(6, address);
  ASSERT_FALSE(symbols_add(&table, lookup, 99));
  ASSERT_EQ(3, table.count);
  ASSERT(symbols_find(&table, lookup, &address));
  ASSERT_EQ(0, address);
  ASSERT_FALSE(symbols_find(&table, lexer_next(&lexer), &address));
  symbols_free(&table);
  ASSERT_EQ(0, table.count);
  ASSERT(table.entries == NULL);
  PASS();
}

TEST resolve_forward_backward_and_data_symbols(void) {
  const char *source = "start: subleq a, b, done\n"
                       "done: subleq b, b, -1\n"
                       "a: .word 3\n"
                       "b: .word 10\n"
                       "pointer: .word start\n"
                       "forward: .word end\n"
                       "alias:\n"
                       "end: .word alias";
  const int64_t expected[] = {6, 7, 3, 7, 7, -1, 3, 10, 0, 10, 10};
  MachineCode code;
  ASSERT(assemble_source(source, &code));
  ASSERT_EQ(sizeof(expected) / sizeof(expected[0]), code.count);
  ASSERT_MEM_EQ(expected, code.words, sizeof(expected));
  free(code.words);
  PASS();
}

TEST assemble_numeric_words_and_limits(void) {
  const char *source = "subleq 9, 10, -1\n"
                       ".word -9223372036854775808\n"
                       ".word 9223372036854775807";
  const int64_t expected[] = {9, 10, -1, INT64_MIN, INT64_MAX};
  MachineCode code;
  ASSERT(assemble_source(source, &code));
  ASSERT_EQ(5, code.count);
  ASSERT_MEM_EQ(expected, code.words, sizeof(expected));
  free(code.words);
  PASS();
}

TEST labels_do_not_emit_words(void) {
  MachineCode code;
  ASSERT(assemble_source("; empty\n", &code));
  ASSERT_EQ(0, code.count);
  ASSERT(code.words == NULL);
  ASSERT(assemble_source("first:\nsecond:", &code));
  ASSERT_EQ(0, code.count);
  ASSERT(code.words == NULL);
  ASSERT(assemble_source(".word end\nend:", &code));
  ASSERT_EQ(1, code.count);
  ASSERT_EQ(1, code.words[0]);
  free(code.words);
  PASS();
}

TEST assembly_errors_leave_output_unchanged(void) {
  const char *sources[] = {
      "same: .word 1\nsame: .word 2",
      "subleq missing, 0, -1",
      "subleq 0, missing, -1",
      "subleq 0, 0, missing",
      ".word missing",
      "subleq 1, 2",
  };
  for (size_t i = 0; i < sizeof(sources) / sizeof(sources[0]); ++i) {
    int64_t sentinel = 123;
    MachineCode code = {&sentinel, 1};
    ASSERT_FALSE(assemble_source(sources[i], &code));
    ASSERT(code.words == &sentinel);
    ASSERT_EQ(1, code.count);
    ASSERT_EQ(123, sentinel);
  }
  PASS();
}

TEST assembled_program_runs_in_vm(void) {
  const char *source = "loop: subleq one, counter, done\n"
                       "subleq zero, zero, loop\n"
                       "done: subleq zero, zero, -1\n"
                       "one: .word 1\n"
                       "counter: .word 3\n"
                       "zero: .word 0";
  MachineCode code;
  ASSERT(assemble_source(source, &code));
  ASSERT_EQ(12, code.count);
  SubleqVM vm = {0};
  memcpy(vm.memory, code.words, code.count * sizeof(int64_t));
  free(code.words);
  vm_run(&vm);
  ASSERT(vm.halted);
  ASSERT_EQ(-1, vm.pc);
  ASSERT_EQ(0, vm.memory[10]);
  ASSERT_EQ(1, vm.memory[9]);
  PASS();
}

SUITE(symbols) { RUN_TEST(symbol_lookup_compares_text_and_length); }

SUITE(assembler) {
  RUN_TEST(resolve_forward_backward_and_data_symbols);
  RUN_TEST(assemble_numeric_words_and_limits);
  RUN_TEST(labels_do_not_emit_words);
  RUN_TEST(assembly_errors_leave_output_unchanged);
  RUN_TEST(assembled_program_runs_in_vm);
}

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(symbols);
  RUN_SUITE(assembler);
  GREATEST_MAIN_END();
}
