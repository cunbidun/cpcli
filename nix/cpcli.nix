{
  pkgs,
  cpcliSrc,
  tauriTaskEditor,
}:
pkgs.stdenv.mkDerivation {
  pname = "cpcli";
  version = "0.6.0";
  src = cpcliSrc;

  nativeBuildInputs = with pkgs; [
    cmake
    ninja
    makeWrapper
  ];

  buildInputs = with pkgs; [
    boost
    cli11
    crow
    gtest
    inja
    nlohmann_json
    spdlog
    toml11
  ];

  cmakeFlags = [
    "-DBUILD_TESTING=ON"
  ];

  doCheck = true;

  postInstall = ''
    rm -f "$out/bin/cpcli_editor"
    rm -f "$out/share/cpcli/task-editor/task-editor"
    install -Dm755 ${tauriTaskEditor}/bin/task-editor \
      "$out/share/cpcli/task-editor/tauri_task_editor"
    ln -s tauri_task_editor "$out/share/cpcli/task-editor/task-editor"
    ln -s ../share/cpcli/task-editor/tauri_task_editor "$out/bin/cpcli_editor"

    wrapProgram "$out/bin/cpcli_app" \
      --set CPCLI_DATA_DIR "$out/share/cpcli"
    wrapProgram "$out/bin/cpcli_cc" \
      --set CPCLI_DATA_DIR "$out/share/cpcli"
  '';
}
