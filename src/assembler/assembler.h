#pragma once

#include "parser.h"

typedef struct {
  int64_t *words;
  size_t count;
} MachineCode;

bool assemble(const Program *program, MachineCode *out);
