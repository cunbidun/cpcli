{
  pkgs,
  cpcliSrc,
}:
let
  bazelStartupFlags = [
    "--nosystem_rc"
    "--nohome_rc"
    "--bazelrc=/dev/null"
    "--output_user_root=$TMPDIR/bazel-root"
  ];

  bazelCommandFlags = [
    "--repository_cache=$TMPDIR/repository-cache"
    "--experimental_repository_cache_hardlinks=false"
    "--lockfile_mode=error"
    "--java_header_compilation=false"
    "--nouse_ijars"
    "--java_runtime_version=local_jdk"
    "--tool_java_runtime_version=local_jdk"
  ];

  bazelNativeBuildInputs = with pkgs; [
    bazel_8
    zulu
    gcc
    git
    gnutar
    gzip
    unzip
    zip
    tree
    cacert
  ];
in rec {
  inherit bazelCommandFlags bazelNativeBuildInputs bazelStartupFlags;

  bazel-deps = pkgs.stdenvNoCC.mkDerivation {
    pname = "cpcli-bazel-deps";
    version = "0.0.0";
    src = cpcliSrc;

    nativeBuildInputs = bazelNativeBuildInputs;

    outputHashMode = "recursive";
    outputHashAlgo = "sha256";
    outputHash = "sha256-COnhWFVGcAz70OmmF9XLHZS+Tr5HQG/N8Fru9n99W9I=";

    dontConfigure = true;
    dontInstall = true;

    buildPhase = ''
      runHook preBuild

      export HOME=$TMPDIR/home
      export JAVA_HOME=${pkgs.zulu.home}
      export SSL_CERT_FILE=${pkgs.cacert}/etc/ssl/certs/ca-bundle.crt
      export GIT_SSL_CAINFO=$SSL_CERT_FILE
      mkdir -p "$HOME" "$TMPDIR/repository-cache" "$TMPDIR/bazel-root"

      bazel ${pkgs.lib.concatStringsSep " " bazelStartupFlags} fetch ${pkgs.lib.concatStringsSep " " bazelCommandFlags} //:assemble_install_tree_nix
      bazel ${pkgs.lib.concatStringsSep " " bazelStartupFlags} shutdown || true

      mkdir -p "$out/repository-cache"
      cp -a "$TMPDIR/repository-cache/." "$out/repository-cache/"

      runHook postBuild
    '';
  };
}
