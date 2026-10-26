#include "greatest.h"
#include "vm/vm_subleq.h"

GREATEST_MAIN_DEFS();

TEST running_a_halted_vm_preserves_state(void) {
  SubleqVM vm = {.memory = {42}, .pc = 12, .halted = 1};
  vm_run(&vm);
  ASSERT_EQ(42, vm.memory[0]);
  ASSERT_EQ(12, vm.pc);
  ASSERT_EQ(1, vm.halted);
  PASS();
}

TEST subtract_two_positive_numbers(void) {
  SubleqVM vm = {.memory = {10, 11, 24, [10] = 17, [11] = 42}};
  vm_step(&vm);
  ASSERT_EQ(25, vm.memory[11]);
  PASS();
}

TEST subtract_and_branch_when_negative(void) {
  SubleqVM vm = {.memory = {10, 11, 24, [10] = 50, [11] = 42}};
  vm_step(&vm);
  ASSERT_EQ(-8, vm.memory[11]);
  ASSERT_EQ(24, vm.pc);
  PASS();
}

TEST branch_when_zero(void) {
  SubleqVM vm = {.memory = {10, 11, 24, [10] = 42, [11] = 42}};
  vm_step(&vm);
  ASSERT_EQ(0, vm.memory[11]);
  ASSERT_EQ(24, vm.pc);
  PASS();
}

TEST continue_when_positive(void) {
  SubleqVM vm = {.memory = {10, 11, 24, [10] = 1, [11] = 2}};
  vm_step(&vm);
  ASSERT_EQ(3, vm.pc);
  PASS();
}

TEST unconditional_branch(void) {
  SubleqVM vm = {.memory = {10, 10, 24, [10] = 7}};
  vm_step(&vm);
  ASSERT_EQ(24, vm.pc);
  PASS();
}

TEST halt_with_negative_destination(void) {
  SubleqVM vm = {.memory = {10, 10, -1}};
  vm_run(&vm);
  ASSERT(vm.halted);
  ASSERT_EQ(-1, vm.pc);
  PASS();
}

TEST reject_invalid_memory_address(void) {
  SubleqVM vm = {.memory = {MEMORY_SIZE, 10, -1}};
  vm_step(&vm);
  ASSERT(vm.halted);
  PASS();
}

TEST reject_arithmetic_overflow(void) {
  SubleqVM vm = {.memory = {10, 11, -1, [10] = -1, [11] = INT64_MAX}};
  vm_step(&vm);
  ASSERT(vm.halted);
  ASSERT_EQ(INT64_MAX, vm.memory[11]);
  PASS();
}

TEST self_modifying_instruction(void) {
  // The first instruction changes the next instruction's source from 10 to 9.
  SubleqVM vm = {.memory = {10, 3, 3, 10, 11, -1, [9] = 2, 1, 5}};
  vm_step(&vm);
  ASSERT_EQ(9, vm.memory[3]);
  vm_step(&vm);
  ASSERT_EQ(3, vm.memory[11]);
  PASS();
}

TEST stop_infinite_loop_at_instruction_limit(void) {
  SubleqVM vm = {0}; // SUBLEQ 0, 0, 0 loops forever without a limit.
  vm_run(&vm);
  ASSERT_EQ(0, vm.pc);
  PASS();
}

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_TEST(running_a_halted_vm_preserves_state);
  RUN_TEST(subtract_two_positive_numbers);
  RUN_TEST(subtract_and_branch_when_negative);
  RUN_TEST(branch_when_zero);
  RUN_TEST(continue_when_positive);
  RUN_TEST(unconditional_branch);
  RUN_TEST(halt_with_negative_destination);
  RUN_TEST(reject_invalid_memory_address);
  RUN_TEST(reject_arithmetic_overflow);
  RUN_TEST(self_modifying_instruction);
  RUN_TEST(stop_infinite_loop_at_instruction_limit);

  GREATEST_MAIN_END();
}
