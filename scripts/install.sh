#!/usr/bin/env bash

set -e

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

# java_task_editor_path=$(rlocation "cpcli/default/task_editor/java_task_editor/java_task_editor_deploy.jar")
cli_task_editor_path=$(rlocation "cpcli/default/task_editor/cli_task_editor/cli_task_editor")

cpp_template_dir=$(rlocation "cpcli/default/templates/cpp")
py_template_dir=$(rlocation "cpcli/default/templates/py")
java_template_dir=$(rlocation "cpcli/default/templates/java")
rs_template_dir=$(rlocation "cpcli/default/templates/rs")
common_template_dir=$(rlocation "cpcli/default/templates/common")

# check if variable $OUT is set 
if [ -z "$OUT" ]; then
	echo "ERROR: OUT variable is not set"
	exit 1;
fi

# check if $OUT is a directory
if  [ ! -d "$OUT" ]; then
	echo "ERROR: $OUT is not a directory"
	exit 1;
fi

mkdir -p "$OUT/bin"
mkdir -p "$OUT/share/cpcli"
# --- Remove leftover files ---
echo "Removing leftover files..."
rm -rf "$OUT/share/cpcli"
rm -f "$OUT/bin/cpcli_app"
rm -f "$OUT/bin/cpcli_cc"
rm -f "$OUT/bin/cpcli_editor"

# --- copy binaries to ~/.local/bin ---
echo "Copy cpcli_app binary to $OUT/bin"
cp "$cpcli_app_path" "$OUT/bin"

echo "Copy cpcli_cc binary to $OUT/bin"
cp "$cpcli_cc_path" "$OUT/bin"

# --- copy artifacts to ~/.local/share ---
echo "Cleanup the cpcli artifacts directory at $OUT/share/cpcli"
mkdir -p "$OUT/share/cpcli"

echo "Create checkers directory at $OUT/share/cpcli/checkers"
mkdir -p "$OUT/share/cpcli/checkers"

echo "Copy checkers binaries to in $OUT/share/cpcli/checkers"
cp "$double_4_path" "$OUT/share/cpcli/checkers"
cp "$double_6_path" "$OUT/share/cpcli/checkers"
cp "$double_9_path" "$OUT/share/cpcli/checkers"
cp "$token_checker_path" "$OUT/share/cpcli/checkers"

echo "Create checkers directory at $OUT/share/cpcli/task-editor"
mkdir -p "$OUT/share/cpcli/task-editor"
# echo "Copying java test editor"
# cp "$java_task_editor_path" "$OUT/share/cpcli/task-editor/java-task-editor.jar"
echo "Copying cli test editor"
cp "$cli_task_editor_path" "$OUT/share/cpcli/task-editor/cli_task_editor"

echo "Create templates directory at $OUT/share/cpcli/templates"
mkdir -p "$OUT/share/cpcli/templates"

cp -r "$cpp_template_dir" "$OUT/share/cpcli/templates"
cp -r "$py_template_dir" "$OUT/share/cpcli/templates"
cp -r "$java_template_dir" "$OUT/share/cpcli/templates"
cp -r "$rs_template_dir" "$OUT/share/cpcli/templates"
cp -r "$common_template_dir" "$OUT/share/cpcli/templates"

echo "Current tree"
tree "$OUT/share/cpcli/"
