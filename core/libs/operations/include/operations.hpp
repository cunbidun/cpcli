#ifndef _cpcli_operations_hpp_
#define _cpcli_operations_hpp_

#include "CLI/CLI.hpp"
#include "nlohmann/json.hpp"
#include "spdlog/spdlog.h"
#include "template_manager.hpp"
#include "utils.hpp"

#include <filesystem>
#include <iostream>
#include <optional>
#include <string>
#include <unistd.h>
#include <vector>

// TODO add docs
bool check_file(std::filesystem::path path, const std::string &error_message);

// TODO add docs
nlohmann::json read_problem_config(std::filesystem::path path, std::filesystem::path temp_config_path);

// TODO add docs
nlohmann::json read_project_config(std::filesystem::path path);

int create_new_task(nlohmann::json project_conf);

void print_duration(std::chrono::high_resolution_clock::time_point t_start);

// TODO add docs
void print_report(const std::string report_name, bool passed, bool rte, bool tle, bool wa, long long runtime);

// TODO add docs
void edit_config(std::filesystem::path root_dir, TemplateManager &template_manager, std::string &frontend_exec);

bool compile_headers(std::filesystem::path precompiled_dir,
                     const std::string &cc,
                     const std::string &flag,
                     const std::string &debug);

enum class ParserOperations {
  Archive,
  Build,
  BuildWithTerm,
  BuildWithDebug,
  NewTask,
  GenHeader,
  EditTaskConfig,
};

class ParserReturnValues {
public:
  std::optional<std::filesystem::path> root_dir;
  std::filesystem::path project_config_path;
  ParserOperations operation;
};

ParserReturnValues parse_args(int argc, char *argv[]);
#endif
