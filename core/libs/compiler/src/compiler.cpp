#include "compiler.hpp"
#include "core/libs/utils/include/utils.hpp"
#include "path_manager.hpp"
#include "spdlog/spdlog.h"

#include <filesystem>

Compiler::Compiler(json project_conf, PathManager &path_manager, std::filesystem::path root_dir, bool is_debug)
    : project_config(project_conf), root_dir(root_dir), path_manager(path_manager), is_debug(is_debug) {}

int Compiler::compile_cpp(std::filesystem::path path, bool is_solution_file) {
  /*
   * Example of cpp config:
   *
   *  "[cpp]": {
   *    "compiler": "clang++",
   *    "regular_flag": "-DLOCAL -O2 -std=c++17",
   *    "debug_flag": "-DLOCAL -Wall -Wshadow -std=c++17 -g -fsanitize=address -fsanitize=undefined -D_GLIBCXX_DEBUG",
   *    "use_precompiled_header": false,
   *    "use_cache": true,
   *  },
   */
  spdlog::debug(
      "compile_cpp: '{}'. is_solution_file: '{}'. debug is '{}'", path.generic_string(), is_solution_file, is_debug);
  std::string language = "[cpp]";
  auto language_config = project_config["language_config"][language];
  auto cpp_compiler = language_config["compiler"].get<std::string>();
  spdlog::debug("cpp compiler is: {}", cpp_compiler);
  auto cache_dir = path_manager.get_cache_dir(root_dir);
  spdlog::debug("cache dir is: {}", cache_dir.generic_string());
  bool use_cache = language_config["use_cache"].get<bool>();

  std::string binary_name = path.stem();
  std::filesystem::path binary_cache_dir = cache_dir / binary_name;
  std::filesystem::path file_cache_dir = cache_dir / path.filename();
  if (use_cache && compare_files(path, file_cache_dir) && (!is_debug || !is_solution_file)) {
    // Use cache when when use_cache is true and the content are the same.
    // However, if the filetype is solution and we are debuging, cache will be disable
    spdlog::debug("the file is not changed, use cache");
    std::filesystem::copy_file(
        binary_cache_dir, path.parent_path() / binary_name, std::filesystem::copy_options::overwrite_existing);
    return 0;
  } else {
    spdlog::debug("the file is changed, not use cache");
    std::string compiler_flags;
    if (language_config["use_precompiled_header"]) {
      std::filesystem::path precompiled_dir = path_manager.get_local_share() / "precompiled_headers";
      std::filesystem::path precompiled_path = precompiled_dir / "cpp_compile_flag" / "stdc++.h";
      check_file(precompiled_dir / "cpp_compile_flag" / "stdc++.h.gch",
                 "precompiled header not found! Please try 'cpcli_app project -g'");

      std::filesystem::path precompiled_debug_path = precompiled_dir / "cpp_debug_flag" / "stdc++.h";
      check_file(precompiled_dir / "cpp_debug_flag" / "stdc++.h.gch",
                 "precompiled debug header not found! Please try 'cpcli_app project -g'");

      language_config["regular_flag"] =
          language_config["regular_flag"].get<string>() + " " + "-include" + " \"" + precompiled_path.string() + "\"";
      language_config["debug_flag"] = language_config["debug_flag"].get<string>() + " " + "-include" + " \"" +
                                      precompiled_debug_path.string() + "\"";
    }
    if (is_debug && is_solution_file) {
      compiler_flags = language_config["debug_flag"].get<std::string>();
      use_cache = false;
    } else {
      compiler_flags = language_config["regular_flag"].get<std::string>();
    }

    if (!is_solution_file && path_manager.has_customize_include_dir()) {
      // do not add include dir for solution filetype
      string include_dir = "\"" + path_manager.get_include().generic_string() + "\"";
      std::vector<string> command{compiler_flags, "-I", include_dir};
      compiler_flags = join(command);
    }

    std::filesystem::current_path(path.parent_path());
    std::vector<std::string> command;

    // build command
    command.push_back(cpp_compiler);
    command.push_back(compiler_flags);
    command.push_back("-o");
    command.push_back(binary_name);
    command.push_back("\"" + path.string() + "\"");

    int status = system_warper(join(command));

    if (status != 0) {
      clean_up();
      exit(CompilerError);
    }
  }

  if (is_solution_file) {
    std::filesystem::copy_file(
        path, path_manager.get_output() / "solution.cpp", std::filesystem::copy_options::overwrite_existing);
  }

  if (use_cache) {
    std::filesystem::copy_file(
        path.parent_path() / binary_name, binary_cache_dir, std::filesystem::copy_options::overwrite_existing);
    std::filesystem::copy_file(path, file_cache_dir, std::filesystem::copy_options::overwrite_existing);
  }

  return 0;
}

int Compiler::compile_python(std::filesystem::path path, bool is_solution_file) {
  spdlog::debug(
      "compile_python: '{}'. is_solution_file: '{}'. debug is '{}'", path.generic_string(), is_solution_file, is_debug);

  std::string language = "[py]";
  auto language_config = project_config["language_config"][language];
  auto compiler = language_config["compiler"].get<std::string>();

  std::string command = compiler + " -m py_compile " + "\"" + path.generic_string() + "\"";
  spdlog::debug("check python compile error with command '{}'", command);
  if (system_warper(command) != 0) {
    clean_up();
    exit(CompilerError);
  }

  std::ofstream file(path.parent_path() / path.stem());
  file << "#!/bin/sh" << std::endl;
  file << compiler << " " << path.filename().generic_string() << std::endl;
  file.close();

  std::filesystem::permissions(
      path.parent_path() / path.stem(), std::filesystem::perms::owner_exec, std::filesystem::perm_options::add);

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
