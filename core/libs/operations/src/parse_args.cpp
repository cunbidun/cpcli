#include "constant.hpp"
#include "operations.hpp"

ParserReturnValues parse_args(int argc, char *argv[]) {
  CLI::App parser{"Competitive Programming CLI"};
  parser.footer("Check out https://github.com/cunbidun/cpcli for more info.");

  ParserReturnValues return_value;

  auto convert_to_canonical = [](std::filesystem::path path) { return std::filesystem::canonical(path); };

  /**
   *  General options and required options
   */
  parser.set_version_flag("-v,--version", VERSION);
  parser.set_help_flag("-h,--help", "Print this help message and exit");
  parser.set_help_all_flag("-H,--help-all", "Print subcommand help and exit");
  parser.add_flag_function(
      "-d,--debug",
      [](int count) {
        spdlog::set_level(spdlog::level::debug);
        spdlog::debug("Debug cpcli (--debug) is set");
      },
      "Run cpcli_app with debug flags (this option will print debug logs)");
  parser.add_option("-p,--project-config", return_value.project_config_path, "Path to the project config file, default to $PWD/project_config.json")
      ->default_val(std::filesystem::current_path() / "project_config.json")
      ->check(CLI::ExistingFile)
      ->transform(convert_to_canonical);

  /**
   *  Project operations
   */
  auto project_subcommand = parser.add_subcommand("project", "Operation on project");
  auto project_operations = project_subcommand->add_option_group("project", "Operation on project");
  project_operations->add_flag_function(
      "-g,--gen-headers",
      [&return_value](int count) {
        spdlog::debug("Generate headers (--gen-headers) is set");
        return_value.operation = ParserOperations::GenHeader;
      },
      "Precompile headers file. This needs to be done before using the precompiled headers (only works on Linux)");
  project_operations->add_flag_function(
      "-n,--new-task",
      [&return_value](int count) {
        spdlog::debug("New task (--new-task) is set");
        return_value.operation = ParserOperations::NewTask;
      },
      "Create a new task");
  project_operations->require_option(1);

  /**
   * Task operation
   */
  auto task_subcommand = parser.add_subcommand("task", "Operation on task");
  task_subcommand->add_option("-r,--root-dir", return_value.root_dir, "The task root directory")
      ->required(true)
      ->check(CLI::ExistingDirectory)
      ->transform(convert_to_canonical);
  auto task_operations = task_subcommand->add_option_group("task", "Operation on task");
  task_operations->add_flag_function(
      "-a,--archive",
      [&return_value](int count) {
        spdlog::debug("Archive task (--archive) is set");
        return_value.operation = ParserOperations::Archive;
      },
      "Archive current task");
  task_operations->add_flag_function(
      "-b,--build",
      [&return_value](int count) {
        spdlog::debug("Build task (--build) is set");
        return_value.operation = ParserOperations::Build;
      },
      "Build task with normal flags");

  task_operations->add_flag_function(
      "-t,--build-with-term",
      [&return_value](int count) {
        spdlog::debug("Build task with terminal (--build-with-term) is set");
        return_value.operation = ParserOperations::BuildWithTerm;
      },
      "Build solution file with terminal (this option will not use cpcli_app)");
  task_operations->add_flag_function(
      "-w,--build-with-debug",
      [&return_value](int count) {
        spdlog::debug("Build task with debug flags (--build-with-debug) is set");
        return_value.operation = ParserOperations::BuildWithDebug;
      },
      "Build task with debug flags");
  task_operations->add_flag_function(
      "-e,--edit-problem-config",
      [&return_value](int count) {
        spdlog::debug("Build task with debug flags (--build-with-debug) is set");
        return_value.operation = ParserOperations::EditTaskConfig;
      },
      "Edit the problem config file, updating the config.json");
  task_operations->require_option(1);
  project_subcommand->excludes(task_subcommand);
  parser.require_subcommand(1);

  try {
    parser.parse(argc, argv);
    spdlog::debug("Parsing command line arguments done");
    spdlog::debug("Project config path: {}", return_value.project_config_path.generic_string());
    if (return_value.root_dir) {
      spdlog::debug("Task root_dir path: {}", (*return_value.root_dir).generic_string());
    }
  } catch (const CLI::ParseError &e) {
    parser.exit(e);
    exit(1);
  }
  return return_value;
}