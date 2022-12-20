#ifndef _cpcli_compiler_hpp_
#define _cpcli_compiler_hpp_

#include "nlohmann/json.hpp"
#include "operations.hpp"
#include "path_manager.hpp"
#include "utils.hpp"

const int static CompilerError = 120000;
const int static CompilerLanguageNotSuported = 120001;
const int static CompilerRequestedFileIsNotRegular = 120002;

class Compiler {
  using json = nlohmann::json;

public:
  Compiler(json project_conf, PathManager &path_manager, std::filesystem::path root_dir, bool is_debug);
  int compile(std::filesystem::path path);

  // methods to compile specific languages
  // the solution filetype will receives special treatments
  int compile_cpp(std::filesystem::path path, bool is_solution_file);
  int compile_python(std::filesystem::path path, bool is_solution_file);
  int compile_java(std::filesystem::path path, bool is_solution_file);

private:
  json project_config;
  std::filesystem::path root_dir;
  PathManager path_manager;
  bool is_debug;
};

#endif
