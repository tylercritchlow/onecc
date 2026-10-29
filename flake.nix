{
  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";
  inputs.greatest = {
    url = "github:silentbicycle/greatest";
    flake = false;
  };

  outputs =
    {
      self,
      nixpkgs,
      greatest,
    }:
    let
      systems = [
        "aarch64-darwin"
        "x86_64-darwin"
        "aarch64-linux"
        "x86_64-linux"
      ];
      forAllSystems = nixpkgs.lib.genAttrs systems;
    in
    {
      packages = forAllSystems (
        system:
        let
          pkgs = nixpkgs.legacyPackages.${system};
        in
        {
          default = pkgs.stdenv.mkDerivation {
            pname = "onecc";
            version = "0.1.0";
            outputs = [
              "out"
              "tests"
            ];
            cmakeFlags = [
              "-DONECC_TEST_INSTALL_DIR=${placeholder "tests"}/bin"
              "-DGREATEST_INCLUDE_DIR=${greatest}"
            ];

            src = pkgs.lib.cleanSourceWith {
              src = ./.;
              filter = path: type: pkgs.lib.cleanSourceFilter path type && baseNameOf path != "build";
            };
            nativeBuildInputs = [ pkgs.cmake ];
            doCheck = true;
            checkPhase = ''
              runHook preCheck
              ctest --output-on-failure
              runHook postCheck
            '';
          };
        }
      );

      checks = forAllSystems (system: {
        tests = self.packages.${system}.default;
      });

      apps = forAllSystems (system: let
        pkgs = nixpkgs.legacyPackages.${system};
        tests = self.packages.${system}.default.tests;
        runner = pkgs.writeShellScript "onecc-tests" ''
          status=0
          "${tests}/bin/test_vm" "$@" || status=1
          "${tests}/bin/test_lexer" "$@" || status=1
          "${tests}/bin/test_parser" "$@" || status=1
          exit "$status"
        '';
      in {
        test = {
          type = "app";
          meta.description = "Run all onecc tests";
          program = "${runner}";
        };
      });

      formatter = forAllSystems (
        system:
        let
          pkgs = nixpkgs.legacyPackages.${system};
          config = pkgs.writeText "treefmt.toml" ''
            excludes = ["build/**", "result/**", "result-*/**", "vendor/**", ".cache/**"]

            [formatter.nix]
            command = "nixfmt"
            includes = ["*.nix"]

            [formatter.c]
            command = "clang-format"
            options = ["-i", "--style=LLVM"]
            includes = ["*.c", "*.h"]
          '';
        in
        pkgs.writeShellApplication {
          name = "onecc-fmt";
          runtimeInputs = [
            pkgs.treefmt
            pkgs.nixfmt
            pkgs.clang-tools
          ];
          text = ''
            exec treefmt --config-file ${config} --tree-root . "$@"
          '';
        }
      );

      devShells = forAllSystems (
        system:
        let
          pkgs = nixpkgs.legacyPackages.${system};
        in
        {
          default = pkgs.mkShell {
            inputsFrom = [ self.packages.${system}.default ];
            packages = [
              pkgs.helix
              (pkgs.writeShellScriptBin "clangd" ''
                exec ${pkgs.clang-tools}/bin/clangd \
                  --query-driver="${pkgs.stdenv.cc}/bin/*" "$@"
              '')
            ];
            shellHook = ''
              cmake -S . -B build/dev \
                -DCMAKE_C_COMPILER=${pkgs.stdenv.cc}/bin/cc \
                -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
                -DGREATEST_INCLUDE_DIR=${greatest}
            '';
          };
        }
      );
    };
}
