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
        || base == "build"
        || base == "result");
}
