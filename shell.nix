{ pkgs ? import <nixpkgs> { } }:
pkgs.mkShell {
  # nativeBuildInputs is usually what you want -- tools you need to run
  nativeBuildInputs = with pkgs.buildPackages; [
    pkgs.bazel_7
    pkgs.glibc
    pkgs.gcc
    pkgs.zulu
  ];
}
