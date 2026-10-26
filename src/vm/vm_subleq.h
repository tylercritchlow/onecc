#include <stdint.h>

#define MEMORY_SIZE 4096

typedef struct {
    int64_t memory[MEMORY_SIZE];
    int64_t pc;
    int halted;
} SubleqVM;

void vm_step(SubleqVM *vm);
void vm_run(SubleqVM *vm);