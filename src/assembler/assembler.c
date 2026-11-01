#include "assembler.h"
#include "symbols.h"

#include <stdio.h>
#include <stdlib.h>

static bool first_pass(const Program *program, SymbolTable *symbols,
                       size_t *word_count) {
  size_t address = 0;
  for (size_t i = 0; i < program->count; ++i) {
    const Statement *statement = &program->statements[i];
    if (statement->type == STMT_LABEL) {
      if (!symbols_add(symbols, statement->value.label, (int64_t)address)) {
        return false;
      }
      continue;
    }

    size_t size = statement->type == STMT_INSTRUCTION ? 3 : 1;
    if (address > SIZE_MAX - size || (uintmax_t)address + size > INT64_MAX) {
      fprintf(stderr, "Program too large\n");
      return false;
    }
    address += size;
  }
  *word_count = address;
  return true;
}

static bool resolve_operand(const SymbolTable *symbols, Operand operand,
                            int64_t *out) {
  if (operand.type == OPERAND_INTEGER) {
    *out = operand.value.integer;
    return true;
  }
  Token name = operand.value.symbol;
  if (symbols_find(symbols, name, out)) {
    return true;
  }
  fprintf(stderr, "%zu:%zu: undefined symbol: ", name.line, name.column);
  fwrite(name.start, 1, name.length, stderr);
  fputc('\n', stderr);
  return false;
}

static bool second_pass(const Program *program, const SymbolTable *symbols,
                        int64_t *words) {
  size_t address = 0;
  for (size_t i = 0; i < program->count; ++i) {
    const Statement *statement = &program->statements[i];
    if (statement->type == STMT_LABEL) {
      continue;
    }
    if (statement->type == STMT_DATA) {
      if (!resolve_operand(symbols, statement->value.data, &words[address])) {
        return false;
      }
      address++;
      continue;
    }
    if (!resolve_operand(symbols, statement->value.instruction.a,
                         &words[address])) {
      return false;
    }
    if (!resolve_operand(symbols, statement->value.instruction.b,
                         &words[address + 1])) {
      return false;
    }
    if (!resolve_operand(symbols, statement->value.instruction.c,
                         &words[address + 2])) {
      return false;
    }
    address += 3;
  }
  return true;
}

bool assemble(const Program *program, MachineCode *out) {
  SymbolTable symbols = {0};
  size_t count;
  if (!first_pass(program, &symbols, &count)) {
    symbols_free(&symbols);
    return false;
  }
  if (count > SIZE_MAX / sizeof(int64_t)) {
    fprintf(stderr, "Program too large\n");
    symbols_free(&symbols);
    return false;
  }

  int64_t *words = NULL;
  if (count > 0) {
    words = malloc(count * sizeof(int64_t));
    if (words == NULL) {
      fprintf(stderr, "Out of memory\n");
      symbols_free(&symbols);
      return false;
    }
  }
  bool success = second_pass(program, &symbols, words);
  symbols_free(&symbols);
  if (!success) {
    free(words);
    return false;
  }
  out->words = words;
  out->count = count;
  return true;
}
