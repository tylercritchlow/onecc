#include "assembler/assembler.h"
#include "file.h"
#include "vm/vm_subleq.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "Usage: %s <file>\n", argv[0]);
    return 1;
  }
  char *source = read_file(argv[1]);
  if (source == NULL) {
    return 1;
  }
  Lexer lexer;
  Parser parser;
  Program program;
  lexer_init(&lexer, source);
  parser_init(&parser, &lexer);
  if (!parser_parse(&parser, &program)) {
    free(source);
    return 1;
  }
  MachineCode code;
  if (!assemble(&program, &code)) {
    free(program.statements);
    free(source);
    return 1;
  }
  int status = 0;
  if (code.count > MEMORY_SIZE) {
    fprintf(stderr, "Program exceeds VM memory (%d words)\n", MEMORY_SIZE);
    status = 1;
  } else {
    SubleqVM vm = {0};
    for (size_t i = 0; i < code.count; ++i)
      vm.memory[i] = code.words[i];
    vm_dump(&vm, code.count, "Before execution");
    vm_run(&vm);
    vm_dump(&vm, code.count, "After execution");
    if (!vm.halted) {
      fprintf(stderr, "VM reached its instruction limit\n");
      status = 1;
    }
  }
  free(code.words);
  free(program.statements);
  free(source);
  return status;
}
