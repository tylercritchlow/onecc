# onecc

Run `nix fmt` from the project root to format Nix files with nixfmt and C
sources/headers with clang-format (LLVM style).

Aiming to build a [SUBLEQ](https://en.wikipedia.org/wiki/One-instruction_set_computer) computing environment, including a virtual machine, assembler, and self-hosting C compiler targeting a one-instruction architecture.

You can build/run everything through nix:

```sh
nix build
nix run . -- tests/test.s
nix run .#test
```

The CLI assembles a source file, loads the words into VM memory starting at
address 0, and runs it. Before and after execution it prints the program counter,
halt status, and memory addresses and values. The memory display includes all
loaded words and nonzero cells outside the program. The VM has no program I/O
yet. Programs larger than 4096 words are rejected. Execution stops after
256 instructions if the program has not halted, and the CLI reports an error.

Assembly uses two passes: labels receive word addresses first, then symbolic
operands are resolved. Instructions occupy three words; `.word` occupies one.
Labels occupy no words. Duplicate labels and undefined symbols are errors.
The assembler produces words without enforcing the VM's 4096-word memory limit.

The test command runs all suites. Pass `greatest` options after `--`, for example `nix run .#test -- -v` for
verbose output, `nix run .#test -- -s lexer` to filter by suite (`lexer`,
`parser`, `symbols`, `assembler`, or `vm`), or `nix run .#test -- -t halted` to filter by test name.

Tests use [greatest](https://github.com/silentbicycle/greatest), fetched as a
Nix input and pinned in `flake.lock`.

For a new suite, add an executable and `add_test` entry in `CMakeLists.txt`.
