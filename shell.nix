{ pkgs ? import <nixpkgs> {} }:
 
 (pkgs.buildFHSUserEnv {
   name = "bazel-env";
   targetPkgs = pkgs: [
     pkgs.bazel_7
     pkgs.glibc
     pkgs.gcc
   ];
 }).env