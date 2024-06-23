{
  description = "cunbidun's Comptitive Programming CLI App";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = {
    self,
    nixpkgs,
    flake-utils,
  }:
    flake-utils.lib.eachDefaultSystem (system: let
      pkgs = nixpkgs.legacyPackages.${system};
    in {
      # packages.cpcli = pkgs.stdenv.mkDerivation {
      #   name = "cpcli";
      #   src = ./.;
      #   buildInputs = with pkgs; [
      #     bazel
      #     bazel_7
      #     zulu
      #     gcc
      #     tree
      #   ];
      #   __noChroot = true;
      #   installPhase = ''
      #     export HOME=$(pwd)
      #     export OUT=$out
      #     mkdir -p $out
      #     bazel run //:install 
      #   '';
      # };

      # defaultPackage = self.packages.${system}.cpcli;
      devShell = pkgs.mkShell {
        buildInputs = with pkgs; [
          bazel_7
          zulu
          gcc
          zsh
          # Add other dependencies your project needs
        ];
        shellHook = ''
          export HOME=$(pwd)
          bazel run @hedron_compile_commands//:refresh_all
          exec zsh
        '';
      };
    });
}
