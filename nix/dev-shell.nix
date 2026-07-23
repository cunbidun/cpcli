{pkgs}:
pkgs.mkShell {
  buildInputs = with pkgs; [
    boost
    cli11
    cmake
    crow
    gcc
    gtest
    inja
    ninja
    nlohmann_json
    spdlog
    toml11
    zsh
    python3
  ];

  shellHook = ''
    export CMAKE_EXPORT_COMPILE_COMMANDS=ON
  '';
}
