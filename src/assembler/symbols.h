#pragma once

#include "lexer.h"
#include <stdbool.h>

typedef struct {
  Token name;
  int64_t address;
} Symbol;

typedef struct {
  Symbol *entries;
  size_t count;
} SymbolTable;

bool symbols_find(const SymbolTable *table, Token name, int64_t *address);
bool symbols_add(SymbolTable *table, Token name, int64_t address);
void symbols_free(SymbolTable *table);
