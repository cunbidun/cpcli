# COMPETITIVE PROGRAMMING CLI (C++)

## What is this?

This project provides a set of tools for testing the correctness of solutions in coding competitions.
Because scripts are all command-line interface (CLI), the project is independent of text editors/ IDEs.
[vscode](https://code.visualstudio.com/) and [nvim](https://neovim.io/) are the recommended text editors.

This project is in the early stage of development, so there is a lot of room for improvement.
Nevertheless, it is mature enough for casual coding competition.

## Install

### Requirements

1. [Nix](https://nixos.org/)

### Step by step installation

1. Clone the repo, run `nix build`, then run `./install.sh`.
2. Create your workspace directory with sub directories:
   - task
   - archive
   - output
3. Create new task and solve problem

You can also run cpcli directly from the checkout without installing it:

```bash
nix run . -- --project-config=/path/to/project_config.toml --help
```

### Workspace Configuration

#### Sample Configuration

This file should be named `project_config.toml` and placed directly at the top level of your workspace folder. `root` is optional; when omitted, cpcli uses the directory containing `project_config.toml`. `~` and relative paths are supported.

```toml
task_editor_exec = "cpcli_editor"
use_template_engine = false

[language_config]
default = "cpp"

[language_config.override]

[language_config.cpp]
compiler = "g++"
regular_flag = "-DLOCAL -O2 -std=c++17"
debug_flag = "-DLOCAL -Wall -Wshadow -std=c++17 -g -fsanitize=address -fsanitize=undefined -D_GLIBCXX_DEBUG"

[language_config.cu]
compiler = "cpcli_cuda_compile"
runtime = "cpcli_cuda_run"
regular_flag = "-std=c++17 -O2 -lineinfo -Wno-deprecated-gpu-targets -gencode arch=compute_70,code=compute_70"
debug_flag = "-std=c++17 -O0 -g -lineinfo -Wno-deprecated-gpu-targets -gencode arch=compute_70,code=compute_70"

[language_config.py]
interpreter = "python3"
```

CUDA tasks use `solution.cu`. When `runtime` is set, cpcli compiles a
`solution.cuda-bin` and creates `./solution` as a launcher that passes the CUDA
binary to that runtime. This allows simulator or container backends without
changing the task runner.

The bundled `cpcli_cuda_setup` command extracts the matching CUDA headers and
`libdevice` into `~/.cache/cpcli/cuda-toolkit-12.8` for clangd/editor support.

#### Sample folder structure

```bash
<your workspace>
├── archive
│   ├── Archive
│   ├── AtCoder - ACL Beginner Contest
│   ├── TopCoder SRM #456
│   ├── Topcoder - TCO 2021 Regional Qualifier 1 DIV 1
│   ├── Topcoder 2021 Round 1B
│   ├── Topcoder Open Algo 2019
│   ├── Unsorted
│   └── vnoi.info
├── include
│   ├── genlib.hpp
│   ├── interactive.hpp
│   └── testlib.h
├── output
│   └── solution.cpp
├── project_config.toml
├── task
│   └── F - Keep Connect
│       ├── config.toml
│       └── solution.cpp
└── template
    ├── checker.template
    ├── gen.template
    ├── interactor.template
    └── solution.template

```

Please take a look at the [archive](https://github.com/cunbidun/competitive_programming/tree/master/archive) folder for more information.

#### Sample invocation

```bash
$ cpcli_app --project-config=project_config.toml task --root-dir="/Users/cunbidun/competitive_programming/task/F - Keep Connect" --build
$ cpcli_app --project-config=project_config.toml task --root-dir="task/F - Keep Connect" --build --subtask full
```

| Num | Attribute                | Type     | Description                                                                                          | Default value                                                                                     |
| --- | ------------------------ | -------- | ---------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------- |
| 1   | `task_dir`               | `string` | Directory for storing current task                                                                   |
| 2   | `output_dir`             | `string` | Compiled solution will be copy to this directory (so you know where to look for file for submission) | `""`                                                                                              |
| 3   | `template_dir`           | `string` | Directory for storing template (check the repo `template` folder for more info)                      | `""`                                                                                              |
| 4   | `archive_dir`            | `string` | Directory for archiving completed task (check the repo `archive` folder for more info)               | `""`                                                                                              |
| 5   | `task_editor_exec`       | `string` | Executable frontend to edit task `config.toml` or `config.json`                                      | `cpcli_editor`                                                                                    |
| 6   | `compiler`               | `string` | C++ compiler command or wrapper                                                                      | `g++`                                                                                             |
| 7   | `regular_flag`           | `string` | C++ regular compilation and link flags                                                               | `"-DLOCAL -O2 -std=c++17"`                                                                        |
| 8   | `debug_flag`             | `string` | C++ debug compilation and link flags                                                                 | `"-DLOCAL -Wall -Wshadow -std=c++17 -g -fsanitize=address -fsanitize=undefined -D_GLIBCXX_DEBUG"` |

cpcli passes `regular_flag` and `debug_flag` directly to the configured compiler. Include paths, precompiled headers, and compiler caching should therefore be expressed explicitly in those flags or in the compiler wrapper. Compilation runs from the task directory, so use absolute paths or environment variables for repository-level files.

### Task Configuration

New task configs are written as `config.toml`. Existing flat `config.json` and old flat TOML keys are still accepted, but saving through the task editor writes the new sectioned TOML shape.

```toml
[problem]
name = "E. Trees of Tranquillity"
group = "Codeforces - Codeforces Round #722 (Div. 2)"
url = "https://codeforces.com/contest/1529/problem/E"

[run]
time_limit_ms = 3000
checker = "custom"
interactive = false
stop_on_first_failure = false

[display]
hide_accepted_tests = false
truncate_long_output = false

[generation]
enabled = true
test_count = 10
seed = ""
args = ""
expected_output_from_slow = true

[[subtasks]]
name = "base"
enabled = true
points = 30
gen = "gen_base"

[subtasks.generation]
enabled = true
test_count = 20
seed = "20260726"
expected_output_from_slow = true

[[subtasks]]
name = "full"
enabled = true
points = 70
depends_on = ["base"]
checker = "double_6"
time_limit_ms = 5000

[[tests]]
subtask = "base"
enabled = true
has_expected_output = true
input = "1\n"
output = "1\n"
```

When `[[subtasks]]` is absent, cpcli treats the existing run, generation, and tests as one implicit subtask. Subtask generators and tests use `___test_case/<subtask-index>/`, so subtask names are never interpolated into shell paths. `depends_on` affects scoring only; it does not rerun dependency tests. Focused `--subtask` runs report the selected verdict but do not calculate a score because dependencies are intentionally not run. Scored runs should normally leave `run.stop_on_first_failure = false`, because that option still aborts the entire run on the first failure.

Canonical task config keys:

| Attribute | Type | Description | Default value |
| --- | --- | --- | --- |
| `problem.name` | `string` | Problem name or id. | `""` |
| `problem.group` | `string` | Contest/group name used for archive. | `""` |
| `problem.url` | `string` | Problem URL. | unset |
| `run.time_limit_ms` | `int` | Time limit in milliseconds. | `10000` |
| `run.checker` | `string` | Checker name. | `token_checker` |
| `run.interactive` | `boolean` | Whether the task is interactive. | `false` |
| `run.stop_on_first_failure` | `boolean` | Stop after WA, RTE, or TLE. | `false` |
| `display.hide_accepted_tests` | `boolean` | Hide accepted test details. | `true` |
| `display.truncate_long_output` | `boolean` | Truncate long input/output when printing. | `false` |
| `generation.enabled` | `boolean` | Enable generated tests. | `false` |
| `generation.test_count` | `int` | Number of generated tests. | `0` |
| `generation.seed` | `string` | Generator seed. If empty, cpcli creates one. | `""` |
| `generation.args` | `string` | Extra arguments passed to `gen` after seed and count. | `""` |
| `generation.expected_output_from_slow` | `boolean` | Use `slow` to produce expected output for generated tests. | `false` |
| `subtasks[].name` | `string` | Unique subtask name used by tests and `--subtask`. | `""` for the implicit subtask |
| `subtasks[].enabled` | `boolean` | Include the subtask in a normal run. An explicit `--subtask` selection can still run it. | `true` |
| `subtasks[].points` | `int` | Optional all-or-nothing score. | unset |
| `subtasks[].depends_on` | `string[]` | Scoring dependencies that must also pass. | `[]` |
| `subtasks[].gen` | `string` | Generator source stem. | `gen` |
| `subtasks[].slow` | `string` | Slow-solution source stem. | `slow` |
| `subtasks[].checker` | `string` | Checker override. | `run.checker` |
| `subtasks[].time_limit_ms` | `int` | Time-limit override. | `run.time_limit_ms` |
| `subtasks[].generation` | `table` | Per-subtask generation settings, with the same fields as `generation`. | inherited from `generation` |
| `tests[].enabled` | `boolean` | Whether this sample test runs. | `true` |
| `tests[].subtask` | `string` | Owning subtask name. | first subtask |
| `tests[].has_expected_output` | `boolean` | Whether `output` should be checked. | derived from `output` |
| `tests[].input` | `string` | Test input. | required |
| `tests[].output` | `string` | Expected output. | optional |

Others `checker` included:

1. `double_4`: checking up to 4 decimals digit
2. `double_6`: checking up to 6 decimals digit
3. `double_9`: checking up to 9 decimals digit
4. `token_checker`: compare output files token by token
5. `custom`: user's custom checker

## Development

### Environment Setup

The recommend text editor for developing this project is [vscode](https://code.visualstudio.com/)

1. Enter the development environment and configure the project:

   ```bash
   nix develop
   cmake -S . -B build -G Ninja -DBUILD_TESTING=ON
   ```

2. Build the project. CMake writes `build/compile_commands.json` for editor tooling.

   ```bash
   cmake --build build
   ```

3. Run the tests:

   ```bash
   ctest --test-dir build --output-on-failure
   ```

4. `.vscode` setup

   Create a `.vscode` at the top level directory of the repository

   ```
   .vscode
   ├── c_cpp_properties.json
   └── settings.json
   ```

   `c_cpp_properties.json` content:

   ```json
   {
     "configurations": [
       {
         "name": "cpcli_app",
        "compileCommands": "${workspaceFolder}/build/compile_commands.json"
       }
     ],
     "version": 4
   }
   ```

### Build and Run Task Editor

```bash
nix build
./install.sh
result/share/cpcli/task-editor/task-editor --root ~/competitive_programming/task/<task>
```

### Build and run GTest

```bash
cmake -S . -B build -G Ninja -DBUILD_TESTING=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

### Build code coverage report

Build with coverage instrumentation, then run the tests and generate the report:

```bash
cmake -S . -B build-coverage -G Ninja -DBUILD_TESTING=ON \
  -DCMAKE_CXX_FLAGS="--coverage" -DCMAKE_EXE_LINKER_FLAGS="--coverage"
cmake --build build-coverage
ctest --test-dir build-coverage
lcov --capture --directory build-coverage --output-file coverage.info
genhtml coverage.info --output-directory coverage
open coverage/index.html
```

### Build Documentations

#### Requirements

Make sure to have `sphinx` and `sphinx-rtd-theme` installed

```bash
$ pip install -U sphinx sphinx-rtd-theme
```

#### Build and run docs

```bash
$ cd docs
$ make clean html
$ open build/html/index.html
```

## Credits

1. The project is heavily inspired by [Egor Kulikov](https://github.com/EgorKulikov)'s [idea-chelper](https://github.com/EgorKulikov/idea-chelper).

2. Checkers and generators use [Mike Mirzayanov](https://github.com/MikeMirzayanov)'s [testlib](https://github.com/MikeMirzayanov/testlib)

3. The precompiled header feature is inspired by [Dushyant Singh](https://github.com/dush1729)'s script [Speed-Up-GCC-Compile-Time](https://github.com/dush1729/Speed-Up-GCC-Compile-Time).
