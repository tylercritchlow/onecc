# onecc
Aiming to build a [SUBLEQ](https://en.wikipedia.org/wiki/One-instruction_set_computer) computing environment, including a virtual machine, assembler, and self-hosting C compiler targeting a one-instruction architecture.

You can build/run everything through nix:

```sh
nix build
nix run
nix run .#test
```

Pass `greatest` options after `--`, for example `nix run .#test -- -v` for
verbose output or `nix run .#test -- -t halted` to filter by test name.

Tests use [greatest](https://github.com/silentbicycle/greatest), fetched as a
Nix input and pinned in `flake.lock`.

For a new suite, add an executable and `add_test` entry in `CMakeLists.txt`.
