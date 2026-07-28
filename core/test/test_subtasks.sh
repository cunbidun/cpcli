#!/usr/bin/env bash

set -euo pipefail

cpcli_app="$1"
token_checker="$2"
workdir="$(mktemp -d)"
trap 'rm -rf "$workdir"' EXIT

mkdir -p "$workdir/task/example" "$workdir/archive" "$workdir/output" \
  "$workdir/data/checkers" "$workdir/data/templates/common"
cp "$token_checker" "$workdir/data/checkers/token_checker"
cp "$(dirname "$0")/../../default/templates/common/problem_config.template" \
  "$workdir/data/templates/common/problem_config.template"

cat >"$workdir/project_config.toml" <<EOF
root = "$workdir"
task_editor_exec = "true"

[language_config]
default = "cpp"

[language_config.override]

[language_config.cpp]
compiler = "g++"
regular_flag = "-O2 -std=c++17"
debug_flag = "-std=c++17"
EOF

cat >"$workdir/task/example/config.toml" <<'EOF'
tests = []

[problem]
name = "Subtask integration"
group = "Tests"

[run]
checker = "token_checker"
time_limit_ms = 3000
stop_on_first_failure = false
interactive = false

[display]
hide_accepted_tests = true
truncate_long_output = false

[generation]
enabled = false
test_count = 0
seed = ""
args = ""
expected_output_from_slow = false

[[subtasks]]
name = "alpha"
points = 30
gen = "gen_alpha"

[subtasks.generation]
enabled = true
test_count = 1
seed = "alpha-seed"

[[subtasks]]
name = "beta"
points = 70
depends_on = ["alpha"]
gen = "gen_beta"

[subtasks.generation]
enabled = true
test_count = 1
seed = "beta-seed"
EOF

cat >"$workdir/task/example/solution.cpp" <<'EOF'
#include <iostream>
int main() {
  int value;
  std::cin >> value;
  std::cout << value << '\n';
}
EOF

cat >"$workdir/task/example/gen_alpha.cpp" <<'EOF'
#include <filesystem>
#include <fstream>
int main() {
  if (std::filesystem::current_path().filename() != "0") return 2;
  std::ofstream("S0.in") << "1\n";
  std::ofstream("S0.out") << "1\n";
}
EOF

cat >"$workdir/task/example/gen_beta.cpp" <<'EOF'
#include <filesystem>
#include <fstream>
int main() {
  if (std::filesystem::current_path().filename() != "1") return 2;
  std::ofstream("S0.in") << "2\n";
  std::ofstream("S0.out") << "2\n";
}
EOF

full_output="$workdir/full-output"
if ! CPCLI_DATA_DIR="$workdir/data" "$cpcli_app" -p "$workdir/project_config.toml" \
  task -r "$workdir/task/example" -b >"$full_output"; then
  cat "$full_output" >&2
  exit 1
fi
grep -q "Subtask alpha: accepted" "$full_output"
grep -q "Subtask beta: accepted" "$full_output"
grep -q "Score: 100/100" "$full_output"

focused_output="$workdir/focused-output"
if ! CPCLI_DATA_DIR="$workdir/data" "$cpcli_app" -p "$workdir/project_config.toml" \
  task -r "$workdir/task/example" -b --subtask beta >"$focused_output"; then
  cat "$focused_output" >&2
  exit 1
fi
grep -q "Subtask beta: accepted" "$focused_output"
grep -q "Score: not calculated for --subtask runs" "$focused_output"
if grep -q "Subtask alpha: accepted" "$focused_output"; then
  echo "focused run unexpectedly executed alpha" >&2
  exit 1
fi
