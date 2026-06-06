{
  pkgs,
  bazel-deps,
  bazelCommandFlags,
  bazelNativeBuildInputs,
  bazelStartupFlags,
  cpcliSrc,
  tauriTaskEditor,
}:
pkgs.stdenvNoCC.mkDerivation {
  pname = "cpcli-install";
  version = "0.0.0";
  src = cpcliSrc;

  nativeBuildInputs = bazelNativeBuildInputs;

  dontConfigure = true;
  dontBuild = true;

  installPhase = ''
    runHook preInstall

    export HOME=$TMPDIR/home
    export JAVA_HOME=${pkgs.zulu.home}
    export SSL_CERT_FILE=${pkgs.cacert}/etc/ssl/certs/ca-bundle.crt
    export GIT_SSL_CAINFO=$SSL_CERT_FILE
    export OUT=$out
    export CPCLI_TAURI_TASK_EDITOR_PATH=${tauriTaskEditor}/bin/task-editor
    mkdir -p "$HOME" "$TMPDIR/repository-cache" "$TMPDIR/bazel-root" "$out"
    cp -a ${bazel-deps}/repository-cache/. "$TMPDIR/repository-cache/"
    chmod -R u+w "$TMPDIR/repository-cache"

    bazel ${pkgs.lib.concatStringsSep " " bazelStartupFlags} fetch ${pkgs.lib.concatStringsSep " " bazelCommandFlags} //:assemble_install_tree_nix
    bazel ${pkgs.lib.concatStringsSep " " bazelStartupFlags} build ${pkgs.lib.concatStringsSep " " bazelCommandFlags} --nofetch //:assemble_install_tree_nix
    bash bazel-bin/assemble_install_tree_nix

    runHook postInstall
  '';
}
