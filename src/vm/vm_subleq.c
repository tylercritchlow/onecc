#include "vm_subleq.h"
#include <stdio.h>

#define INSTRUCTION_LIM 256

void vm_step(SubleqVM *vm) {
  // if PC is negative, we treat that as if it was a halt instruction.
  if (vm->pc < 0) {
    vm->halted = 1;
    return;
  }

  if (vm->pc > MEMORY_SIZE - 3) {
    printf("pc is invalid");
    vm->halted = 1;
    return;
  }

  /*
  The single instruction supported is `SUBLEQ A, B, C` where
  A, B, and C are memory addresses.

  A: Address of the value to subtract (source).
  B: Address of the value to subtract from (destination).
  C: Address of the next instruction if the result is less than or equal to
  zero.
  */

  int64_t A = vm->memory[vm->pc];
  int64_t B = vm->memory[vm->pc + 1];
  int64_t C = vm->memory[vm->pc + 2];

  if (A < 0 || A >= MEMORY_SIZE || B < 0 || B >= MEMORY_SIZE) {
    vm->halted = 1;
    return;
  }

  int64_t source = vm->memory[A];
  int64_t destination = vm->memory[B];
  if ((source > 0 && destination < INT64_MIN + source) ||
      (source < 0 && destination > INT64_MAX + source)) {
    vm->halted = 1;
    return;
  }

  vm->memory[B] = destination - source;

  if (vm->memory[B] <= 0) {
    vm->pc = C;
  } else {
    vm->pc += 3;
  }
}

void vm_run(SubleqVM *vm) {
  uint64_t instructions_ran = 0;

  while (vm->halted != 1 && instructions_ran != INSTRUCTION_LIM) {
    vm_step(vm);
    instructions_ran++;
  }
}
