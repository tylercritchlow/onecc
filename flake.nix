{
  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";
  inputs.greatest = {
    url = "github:silentbicycle/greatest";
    flake = false;
  };

  outputs = { self, nixpkgs, greatest }:
    let
      systems = [ "aarch64-darwin" "x86_64-darwin" "aarch64-linux" "x86_64-linux" ];
      forAllSystems = nixpkgs.lib.genAttrs systems;
    in
    {
      packages = forAllSystems (system:
        let pkgs = nixpkgs.legacyPackages.${system};
        in {
          default = pkgs.stdenv.mkDerivation {
            pname = "onecc";
            version = "0.1.0";
            outputs = [ "out" "tests" ];
            cmakeFlags = [
              "-DONECC_TEST_INSTALL_DIR=${placeholder "tests"}/bin"
              "-DGREATEST_INCLUDE_DIR=${greatest}"
            ];

            src = pkgs.lib.cleanSourceWith {
              src = ./.;
              filter = path: type:
                pkgs.lib.cleanSourceFilter path type
                && baseNameOf path != "build";
            };
            nativeBuildInputs = [ pkgs.cmake ];
            doCheck = true;
            checkPhase = ''
              runHook preCheck
              ctest --output-on-failure
              runHook postCheck
            '';
          };
        });

      checks = forAllSystems (system: {
        tests = self.packages.${system}.default;
      });

      apps = forAllSystems (system: {
        test = {
          type = "app";
          meta.description = "Run the onecc VM tests";
          program = "${self.packages.${system}.default.tests}/bin/test_vm";
        };
      });

      devShells = forAllSystems (system:
        let pkgs = nixpkgs.legacyPackages.${system};
        in {
          default = pkgs.mkShell {
            inputsFrom = [ self.packages.${system}.default ];
          };
        });
    };
}
