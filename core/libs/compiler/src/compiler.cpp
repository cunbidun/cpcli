#include "compiler.hpp"
#include "path_manager.hpp"
#include "spdlog/spdlog.h"
#include "utils.hpp"
#include <inja/inja.hpp>

#include <filesystem>

Compiler::Compiler(json project_conf, PathManager &path_manager, bool is_debug)
    : project_config(project_conf), path_manager(path_manager), is_debug(is_debug) {}

int Compiler::compile_cpp(std::filesystem::path path, bool is_solution_file) {
  /*
   * Example of cpp config:
   *
   *  "[cpp]": {
   *    "compiler": "clang++",
   *    "regular_flag": "-DLOCAL -O2 -std=c++17",
   *    "debug_flag": "-DLOCAL -Wall -Wshadow -std=c++17 -g -fsanitize=address -fsanitize=undefined -D_GLIBCXX_DEBUG",
   *  },
   */
  spdlog::debug(
      "compile_cpp: '{}'. is_solution_file: '{}'. debug is '{}'", path.generic_string(), is_solution_file, is_debug);
  std::string language = "[cpp]";
  auto language_config = project_config["language_config"][language];
  spdlog::debug("language config is {}", language_config.dump());

  auto cpp_compiler = language_config["compiler"].get<std::string>();
  spdlog::debug("cpp compiler is: {}", cpp_compiler);

  // Keep compilation policy in project_config.toml and the compiler wrapper.
  // The separate object step lets wrappers such as ccache cache compilation.
  std::string binary_name = path.stem();
  std::string object_name = "." + binary_name + ".cpcli.o";
  std::string compiler_flags = (is_debug && is_solution_file) ? language_config["debug_flag"].get<std::string>()
                                                              : language_config["regular_flag"].get<std::string>();

  std::filesystem::current_path(path.parent_path());
  {
    inja::Environment env;
    json command_data = {{"cpp_compiler", cpp_compiler},
                         {"compiler_flags", compiler_flags},
                         {"binary_name", binary_name},
                         {"object_name", object_name},
                         {"path", path.generic_string()}};
    std::string compile_command =
        env.render("{{ cpp_compiler }} {{ compiler_flags }} -c \"{{ path }}\" -o \"{{ object_name }}\"", command_data);
    if (system_warper(compile_command) != 0) {
      std::filesystem::remove(object_name);
      clean_up();
      exit(CompilerError);
    }

    std::string link_command = env.render(
        "{{ cpp_compiler }} {{ compiler_flags }} \"{{ object_name }}\" -o \"{{ binary_name }}\"", command_data);
    int link_status = system_warper(link_command);
    std::filesystem::remove(object_name);
    if (link_status != 0) {
      clean_up();
      exit(CompilerError);
    }
  }

  if (is_solution_file) {
    std::filesystem::copy_file(
        path, path_manager.get_output() / "solution.cpp", std::filesystem::copy_options::overwrite_existing);
  }

  return 0;
}

int Compiler::compile_cuda(std::filesystem::path path, bool is_solution_file) {
  spdlog::debug(
      "compile_cuda: '{}'. is_solution_file: '{}'. debug is '{}'", path.generic_string(), is_solution_file, is_debug);

  auto language_config = project_config["language_config"]["[cu]"];
  auto compiler = language_config["compiler"].get<std::string>();
  auto runtime = language_config.value("runtime", std::string());
  auto compiler_flags = is_debug && is_solution_file ? language_config["debug_flag"].get<std::string>()
                                                     : language_config["regular_flag"].get<std::string>();

  auto executable_path = path.parent_path() / path.stem();
  auto cuda_binary_path = executable_path;
  cuda_binary_path += ".cuda-bin";

  std::filesystem::current_path(path.parent_path());
  std::string command_template = "{{ compiler }} {{ compiler_flags }} -o \"{{ binary_name }}\" \"{{ path }}\"";
  inja::Environment env;
  std::string command = env.render(command_template,
                                   {{"compiler", compiler},
                                    {"compiler_flags", compiler_flags},
                                    {"binary_name", cuda_binary_path.filename().generic_string()},
                                    {"path", path.generic_string()}});
  if (system_warper(command) != 0) {
    clean_up();
    exit(CompilerError);
  }

  std::ofstream launcher(executable_path);
  launcher << "#!/bin/sh\n";
  launcher << "script_dir=$(CDPATH= cd -- \"$(dirname -- \"$0\")\" && pwd)\n";
  if (runtime.empty()) {
    launcher << "exec \"$script_dir/" << cuda_binary_path.filename().generic_string() << "\" \"$@\"\n";
  } else {
    launcher << "exec \"" << runtime << "\" \"$script_dir/" << cuda_binary_path.filename().generic_string()
             << "\" \"$@\"\n";
  }
  launcher.close();
  std::filesystem::permissions(executable_path, std::filesystem::perms::owner_exec, std::filesystem::perm_options::add);

  if (is_solution_file) {
    std::filesystem::copy_file(
        path, path_manager.get_output() / "solution.cu", std::filesystem::copy_options::overwrite_existing);
  }
  return 0;
}

int Compiler::compile_python(std::filesystem::path path, bool is_solution_file) {
  spdlog::debug(
      "compile_python: '{}'. is_solution_file: '{}'. debug is '{}'", path.generic_string(), is_solution_file, is_debug);

  std::string language = "[py]";
  auto language_config = project_config["language_config"][language];
  std::string interpreter = language_config["interpreter"].get<std::string>();
  spdlog::debug("interpreter is {}", interpreter);

  {
    // compile and check for syntax error
    std::string command_template = "{{ interpreter }} -m py_compile '{{ path }}'";
    inja::Environment env;
    std::string command = env.render(command_template, {{"interpreter", interpreter}, {"path", path.generic_string()}});
    spdlog::debug("check python compile error with command '{}'", command);
    if (system_warper(command) != 0) {
      clean_up();
      exit(CompilerError);
    }
  }

  std::ofstream file(path.parent_path() / path.stem());
  file << "#!/bin/sh" << std::endl;
  file << interpreter << " " << path.filename().generic_string() << std::endl;
  file.close();

  std::filesystem::permissions(
      path.parent_path() / path.stem(), std::filesystem::perms::owner_exec, std::filesystem::perm_options::add);

  if (is_solution_file) {
    std::filesystem::copy_file(
        path, path_manager.get_output() / "solution.py", std::filesystem::copy_options::overwrite_existing);
  }
  return 0;
}

int Compiler::compile_java(std::filesystem::path path, bool is_solution_file) {
  spdlog::debug(
      "compile_java: '{}'. is_solution_file: '{}'. debug is '{}'", path.generic_string(), is_solution_file, is_debug);

  std::string language = "[java]";
  auto language_config = project_config["language_config"][language];
  std::string compiler = language_config["compiler"].get<std::string>();
  spdlog::debug("compiler is {}", compiler);
  std::string runtime = language_config["runtime"].get<std::string>();
  spdlog::debug("runtime is {}", runtime);

  std::string compiler_flags;
  if (is_debug && is_solution_file) {
    compiler_flags = language_config["debug_flag"].get<std::string>();
  } else {
    compiler_flags = language_config["regular_flag"].get<std::string>();
  }

  std::string command_template = "{{ compiler }} {{ compiler_flags }} '{{ path }}'";
  inja::Environment env;
  std::string command = env.render(
      command_template, {{"compiler", compiler}, {"compiler_flags", compiler_flags}, {"path", path.generic_string()}});
  spdlog::debug("compile java with command '{}'", command);
  if (system_warper(command) != 0) {
    clean_up();
    exit(CompilerError);
  }

  // Generate a shell script to run the class file
  std::string class_name = path.stem();
  std::string file_name = class_name;
  file_name[0] = tolower(class_name[0]);
  std::ofstream file(path.parent_path() / file_name);
  file << "#!/bin/sh" << std::endl;
  file << runtime << " " << class_name << std::endl;
  file.close();

  std::filesystem::permissions(
      path.parent_path() / path.stem(), std::filesystem::perms::owner_exec, std::filesystem::perm_options::add);

  if (is_solution_file) {
    std::filesystem::copy_file(
        path, path_manager.get_output() / "Solution.java", std::filesystem::copy_options::overwrite_existing);
  }
  return 0;
}

int Compiler::compile(std::filesystem::path path) {
  if (std::filesystem::status(path).type() != std::filesystem::file_type::regular) {
    spdlog::error("The path {} is not a path", path.c_str());
    exit(CompilerRequestedFileIsNotRegular);
  }
  auto file_extension = path.extension();
  auto filetype = path.stem();
  spdlog::debug(
      "Compiling '{}'. file_extension '{}'. Filetype '{}'", path.c_str(), file_extension.c_str(), filetype.c_str());
  int status;
  if (file_extension == ".py") {
    status = compile_python(path, filetype == "solution");
  } else if (file_extension == ".cpp") {
    status = compile_cpp(path, filetype == "solution");
  } else if (file_extension == ".cu") {
    status = compile_cuda(path, filetype == "solution");
  } else if (file_extension == ".java") {
    status = compile_java(path, filetype == "Solution");
  } else if (file_extension == ".rs") {
    status = compile_rust(path, filetype == "solution");
  } else {
    spdlog::error("The file extension {} is not supported", file_extension.c_str());
    exit(CompilerLanguageNotSuported);
  }
  if (!std::filesystem::exists(path.parent_path() / path.stem())) {
    spdlog::error("Compile failed. Couldn't file executable file for {}", path.c_str());
    exit(CompilerError);
  }
  return status;
}
