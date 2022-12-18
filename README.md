# COMPETITIVE PROGRAMMING CLI (C++)

## What is this?

This project provides a set of tools for testing the correctness of solutions in coding competitions.
Because scripts are all command-line interface (CLI), the project is independent of text editors/ IDEs.
[vscode](https://code.visualstudio.com/) and [nvim](https://neovim.io/) are the recommended text editors.

This project is in the early stage of development, so there is a lot of room for improvement.
Nevertheless, it is mature enough for casual coding competition.

## Install

### Requirements

1. [Bazel build system](https://bazel.build/)
2. [Java JDK 11+](https://www.java.com/en/) for running the simple task editor interface

### Step by step installation

1. Clone the repo and run `bazel run //:install`.
2. Create your workspace directory with sub directories:
   - task
   - archive
   - output
3. Create new task and solve problem

### Workspace Configuration

#### Sample Configuration

This file should named `project_config.json` and place directly on the top level the your workspace folder.

```json
{
  "root": "/Users/cunbidun/competitive_programming", // your workspace folder
  "task_editor_exec": "java -jar ~/.local/share/cpcli/frontend/TaskConfigEditor.jar",
  "cpp_compiler": "g++",
  "cpp_compile_flag": "-DLOCAL -O2 -std=c++17",
  "cpp_debug_flag": "-DLOCAL -Wall -Wshadow -std=c++17 -g -fsanitize=address -fsanitize=undefined -D_GLIBCXX_DEBUG",
  "use_precompiled_header": false,
  "use_cache": true
}
```

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
├── project_config.json
├── task
│   └── F - Keep Connect
│       ├── config.json
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
$ cpcli_app --project-config=project_config.json task --root-dir="/Users/cunbidun/competitive_programming/task/F - Keep Connect" --build
```

| Num | Attribute                | Type     | Description                                                                                          | Default value                                                                                     |
| --- | ------------------------ | -------- | ---------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------- |
| 1   | `task_dir`               | `string` | Directory for storing current task                                                                   |
| 2   | `output_dir`             | `string` | Compiled solution will be copy to this directory (so you know where to look for file for submission) | `""`                                                                                              |
| 3   | `template_dir`           | `string` | Directory for storing template (check the repo `template` folder for more info)                      | `""`                                                                                              |
| 4   | `archive_dir`            | `string` | Directory for archiving completed task (check the repo `archive` folder for more info)               | `""`                                                                                              |
| 6   | `include_dir`            | `string` | Store libs here                                                                                      |                                                                                                   |
| 5   | `task_editor_exec`          | `string` | Executable frontend to edit task `config.json`                                                       | Ugly java UI                                                                                      |
| 7   | `cpp_compile_flag`       | `string` | Cpp normal complier flag                                                                             | `"-DLOCAL -static -O2 -std=c++17"`                                                                |
| 8   | `cpp_debug_flag`         | `string` | Cpp debug flag                                                                                       | `"-DLOCAL -Wall -Wshadow -std=c++17 -g -fsanitize=address -fsanitize=undefined -D_GLIBCXX_DEBUG"` |
| 9   | `use_precompiled_header` | `bool`   | use precompiled headers                                                                              | `true`                                                                                            |
| 10  | `use_cache`              | `bool`   | Enable cache                                                                                         | `true`                                                                                            |

### Task Configuration

Here is an example: problem [E. Trees of Tranquillity](https://codeforces.com/contest/1529/problem/E) has the following configuration:

```
{
  "timeLimit": 3000,
  "tests": [
    {
      "output": "1",
      "input": "1",
      "index": 0,
      "active": true
    },
    {
      "output": "2",
      "input": "2",
      "index": 1,
      "active": true
    }
  ],
  "name": "E. Trees of Tranquillity",
  "group": "Codeforces - Codeforces Round #722 (Div. 2)"
  "truncateLongTest": false,
  "checker": "custom",

  "url": "https://codeforces.com/contest/1529/problem/E",

  "useGeneration": true,
  "numTest": 10,
  "knowGenAns": true,
  "genParameters": "",
  "generatorSeed": "",

  "interactive": false,
  "stopAtWrongAnswer": false,

  "hideAcceptedTest": false,
}
```

There are 16 attributes:

| Num  | Attribute           | Type                    | Description                                                                                                                                                                                                                             | Default value   |
| ---- | ------------------- | ----------------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | --------------- |
| 1    | `timeLimit`         | `int`                   | Problem's time limit in millisecond (1000ms = 1s)                                                                                                                                                                                       | `10000`         |
| 2    | `tests`             | `array of test objects` | An array to store test cases, each of them is a json object. For each test, the input field is required, but the output field is optional.                                                                                              | `[]`            |
| 3    | `name`              | `string`                | The name of the problem, usually the task's name or id number.                                                                                                                                                                          | `""`            |
| 4    | `group`             | `string`                | The name of the contest, used for archive.                                                                                                                                                                                              | `""`            |
| 5    | `truncateLongTest`  | `boolean`               | Test cases could be really long, so there is high chance that they will cause performance problem. This option is for truncation the test's content when printing. Truncated test will only print the first and the last 35 characters. | `false`         |
| 6 \* | `checker`           | `string`                | Name of the checker                                                                                                                                                                                                                     | `token_checker` |
| 7    | `url`               | `string`                | The url of the problem. And be used for auto submit                                                                                                                                                                                     | `""`            |
| 8    | `useGeneration`     | `boolean`               | For most of problems, sample cases are not enough. This is a tool for generating more test cases.                                                                                                                                       | `false`         |
| 9    | `numTest`           | `number`                | Number of generated test cases.                                                                                                                                                                                                         | `0`             |
| 10   | `knowGenAns`        | `boolean`               | If we have `slow.cpp` for generating test cases output                                                                                                                                                                                  | `false`         |
| 11   | `genParameters`     | `string`                | Parameters for the generator. Currently unused.                                                                                                                                                                                         | `""`            |
| 12   | `generatorSeed`     | `string`                | Generator seed.                                                                                                                                                                                                                         | `""`            |
| 13   | `interactive`       | `boolean`               | For `interactive` problems.                                                                                                                                                                                                             | `false`         |
| 14   | `stopAtWrongAnswer` | `boolean`               | If this option is set to true, the testing process will stop after encounter a `wrong answer`, `rte`, or `tle` test cases.                                                                                                              | `false`         |
| 15   | `hideAcceptedTest`  | `boolean`               | Hide accepted test cases information                                                                                                                                                                                                    | `true`          |

Others `checker` included:

1. `double_4`: checking up to 4 decimals digit
2. `double_6`: checking up to 6 decimals digit
3. `double_9`: checking up to 9 decimals digit
4. `token_checker`: compare output files token by token
5. `custom`: user's custom checker

## Development

### Environment Setup

The recommend text editor for developing this project is [vscode](https://code.visualstudio.com/)

1. Build the project
   ```bash
   bazel run //:install
   ```
2. Build `compile_commands.json` for autocomplete

   ```bash
   bazel run @hedron_compile_commands//:refresh_all
   ```

3. `.vscode` setup

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
         "compileCommands": "${workspaceFolder}/compile_commands.json"
       }
     ],
     "version": 4
   }
   ```

   `settings.json` content:

   ```json
   {
    "java.project.referencedLibraries": [
      "bazel-bin/default/task_editor/java_task_editor/java_task_editor.runfiles/gson/jar/*.jar",
      "bazel-bin/default/task_editor/java_task_editor/java_task_editor.runfiles/common_cli/jar/*.jar"
    ],
   }
   ```

### Build and Run Java Test Editor

```bash
bazel run //default/task_editor/java_task_editor ~/cpcli/default/task_editor
```

### Build and run GTest

```bash
bazel test --test_output=all //core/test:cpcli_test
```

### Build code coverage report

Make sure you have `lcov` installed

```bash
bazel coverage //core/test:cpcli_test --coverage_report_generator="@bazel_tools//tools/test/CoverageOutputGenerator/java/com/google/devtools/coverageoutputgenerator:Main" --instrumentation_filter="//..."
genhtml --output coverage "$(bazel info output_path)/_coverage/_coverage_report.dat"
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
