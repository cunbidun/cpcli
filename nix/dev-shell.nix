{pkgs}:
pkgs.mkShell {
  buildInputs = with pkgs; [
    bazel_8
    zulu
    gcc
    zsh
    python3
  ];

  shellHook = ''
    # Refresh compile_commands.json when in an interactive shell; ignore failures.
    if [ -n "\${PS1:-}" ]; then
      bazel run @hedron_compile_commands//:refresh_all || true
    fi
  '';
}
