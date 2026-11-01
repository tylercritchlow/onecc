#include "symbols.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool symbols_find(const SymbolTable *table, Token name, int64_t *address) {
  for (size_t i = 0; i < table->count; ++i) {
    Token entry = table->entries[i].name;
    if (entry.length == name.length &&
        memcmp(entry.start, name.start, name.length) == 0) {
      *address = table->entries[i].address;
      return true;
    }
  }
  return false;
}

bool symbols_add(SymbolTable *table, Token name, int64_t address) {
  int64_t existing;
  if (symbols_find(table, name, &existing)) {
    fprintf(stderr, "%zu:%zu: duplicate label: ", name.line, name.column);
    fwrite(name.start, 1, name.length, stderr);
    fputc('\n', stderr);
    return false;
  }
  if (table->count >= SIZE_MAX / sizeof(Symbol)) {
    fprintf(stderr, "Too many symbols\n");
    return false;
  }
  Symbol *entries =
      realloc(table->entries, (table->count + 1) * sizeof(Symbol));
  if (entries == NULL) {
    fprintf(stderr, "Out of memory\n");
    return false;
  }
  table->entries = entries;
  table->entries[table->count].name = name;
  table->entries[table->count].address = address;
  table->count++;
  return true;
}

void symbols_free(SymbolTable *table) {
  free(table->entries);
  table->entries = NULL;
  table->count = 0;
}
