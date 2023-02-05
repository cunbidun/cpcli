#include "compiler.hpp"
#include "inja.hpp"
#include "path_manager.hpp"
#include "spdlog/spdlog.h"
#include "utils.hpp"

#include <filesystem>

int Compiler::compile_rust(std::filesystem::path path, bool is_solution_file) {
  spdlog::debug(
      "compile_rust: '{}'. is_solution_file: '{}'. debug is '{}'", path.generic_string(), is_solution_file, is_debug);

  std::string language = "[rs]";
  auto language_config = project_config["language_config"][language];
  std::string compiler = language_config["compiler"].get<std::string>();
  std::string regular_flag = language_config["regular_flag"].get<std::string>();
  spdlog::debug("compiler is {}", compiler);
  spdlog::debug("regular flag is {}", regular_flag);

  {
    // compile and check for syntax error
    std::string command_template = "{{ compiler }} {{ regular_flag }} '{{ path }}'";
    // if (is_debug) {
    //   command_template = "export RUST_BACKTRACE=1; {{ compiler }} {{ regular_flag }} '{{ path }}'";
    // }
    inja::Environment env;
    std::string command = env.render(
        command_template, {{"compiler", compiler}, {"regular_flag", regular_flag}, {"path", path.generic_string()}});
    spdlog::debug("check rust compile error with command '{}'", command);
    if (system_warper(command) != 0) {
      clean_up();
      exit(CompilerError);
    }
  }

  spdlog::debug("pwd is {}", std::filesystem::current_path().generic_string());

  spdlog::debug("compiled successfully! Changing permissions executable");
  std::filesystem::permissions(
      path.parent_path() / path.stem(), std::filesystem::perms::owner_exec, std::filesystem::perm_options::add);

  if (is_solution_file) {
    spdlog::debug("copying source code to output directory");
    std::filesystem::copy_file(
        path, path_manager.get_output() / "solution.rs", std::filesystem::copy_options::overwrite_existing);
  }
  return 0;
}
