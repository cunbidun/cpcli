#!/bin/bash

# --- begin runfiles.bash initialization ---
# Copy-pasted from Bazel's Bash runfiles library (tools/bash/runfiles/runfiles.bash).
set -euo pipefail
if [[ ! -d "${RUNFILES_DIR:-/dev/null}" && ! -f "${RUNFILES_MANIFEST_FILE:-/dev/null}" ]]; then
    if [[ -f "$0.runfiles_manifest" ]]; then
        export RUNFILES_MANIFEST_FILE="$0.runfiles_manifest"
        elif [[ -f "$0.runfiles/MANIFEST" ]]; then
        export RUNFILES_MANIFEST_FILE="$0.runfiles/MANIFEST"
        elif [[ -f "$0.runfiles/bazel_tools/tools/bash/runfiles/runfiles.bash" ]]; then
        export RUNFILES_DIR="$0.runfiles"
    fi
fi
if [[ -f "${RUNFILES_DIR:-/dev/null}/bazel_tools/tools/bash/runfiles/runfiles.bash" ]]; then
    source "${RUNFILES_DIR}/bazel_tools/tools/bash/runfiles/runfiles.bash"
    elif [[ -f "${RUNFILES_MANIFEST_FILE:-/dev/null}" ]]; then
    source "$(grep -m1 "^bazel_tools/tools/bash/runfiles/runfiles.bash " \
    "$RUNFILES_MANIFEST_FILE" | cut -d ' ' -f 2-)"
else
    echo >&2 "ERROR: cannot find @bazel_tools//tools/bash/runfiles:runfiles.bash"
    exit 1
fi
# --- end runfiles.bash initialization ---

# --- declare paths ---
cpcli_app_path=$(rlocation "cpcli/core/bin/cpcli_app")
cpcli_cc_path=$(rlocation "cpcli/core/bin/cpcli_cc")
double_4_path=$(rlocation "cpcli/default/checkers/double_4")
double_6_path=$(rlocation "cpcli/default/checkers/double_6")
double_9_path=$(rlocation "cpcli/default/checkers/double_9")
token_checker_path=$(rlocation "cpcli/default/checkers/token_checker")

# --- copy binaries to ~/.local/bin ---
echo "Copy cpcli_app binary to $HOME/.local/bin"
cp "$cpcli_app_path" "$HOME/.local/bin"

echo "Copy cpcli_cc binary to $HOME/.local/bin"
cp "$cpcli_cc_path" "$HOME/.local/bin"

# --- copy artifacts to ~/.local/bin ---
echo "Cleanup the cpcli artifacts directory in $HOME/.local/share/cpcli"
rm -rf "$HOME/.local/share/cpcli"

echo "Create checkers directory in $HOME/.local/share/cpcli"
mkdir -p "$HOME/.local/share/cpcli/checkers"

echo "Copy checkers binaries to in $HOME/.local/share/cpcli/checkers"
cp "$double_4_path" "$HOME/.local/share/cpcli/checkers"
cp "$double_6_path" "$HOME/.local/share/cpcli/checkers"
cp "$double_9_path" "$HOME/.local/share/cpcli/checkers"
cp "$token_checker_path" "$HOME/.local/share/cpcli/checkers"