# onecc

Run `nix fmt` from the project root to format Nix files with nixfmt and C
sources/headers with clang-format (LLVM style).

Aiming to build a [SUBLEQ](https://en.wikipedia.org/wiki/One-instruction_set_computer) computing environment, including a virtual machine, assembler, and self-hosting C compiler targeting a one-instruction architecture.

You can build/run everything through nix:

```sh
nix build
nix run . -- examples/subtract.s
nix run .#test
```
