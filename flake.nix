{
  description = "cunbidun's Comptitive Programming CLI App";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-26.05";
    flake-utils.url = "github:numtide/flake-utils";
    rust-overlay = {
      url = "github:oxalica/rust-overlay";
      inputs.nixpkgs.follows = "nixpkgs";
    };
  };

  outputs = {
    nixpkgs,
    flake-utils,
    rust-overlay,
    ...
  }:
    flake-utils.lib.eachDefaultSystem (system: let
      pkgs = import nixpkgs {
        inherit system;
        overlays = [(import rust-overlay)];
      };
      root = ./.;
      cpcliSrc = import ./nix/source.nix {inherit pkgs root;};
      tauriTaskEditor = import ./nix/tauri-task-editor.nix {
        inherit pkgs root;
      };
      cpcli = import ./nix/cpcli.nix {
        inherit pkgs cpcliSrc tauriTaskEditor;
      };
    in {
      packages = {
        default = cpcli;
        inherit cpcli;
        tauri-task-editor = tauriTaskEditor;
        install = cpcli;
      };

      apps.default = {
        type = "app";
        program = "${cpcli}/bin/cpcli_app";
        meta.description = "Competitive programming task runner";
      };

      checks.cpcli = cpcli;

      devShells.default = import ./nix/dev-shell.nix {inherit pkgs;};
    });
}
