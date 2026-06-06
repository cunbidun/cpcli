{
  pkgs,
  root,
}:
pkgs.lib.cleanSourceWith {
  src = root;
  filter = path: type:
    let
      base = baseNameOf path;
    in
      !(base == ".git"
        || base == "result"
        || base == "flake.nix"
        || base == "flake.lock"
        || pkgs.lib.hasPrefix "bazel-" base);
}
