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
      bazel = import ./nix/bazel.nix {inherit pkgs cpcliSrc;};
    in {
      packages = {
        inherit (bazel) bazel-deps;
        tauri-task-editor = tauriTaskEditor;
        install = import ./nix/install.nix {
          inherit pkgs cpcliSrc tauriTaskEditor;
          inherit (bazel) bazel-deps bazelCommandFlags bazelNativeBuildInputs bazelStartupFlags;
        };
      };

      devShell = import ./nix/dev-shell.nix {inherit pkgs;};
    });
}
