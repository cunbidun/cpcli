#ifndef _cpcli_path_manager_hpp_
#define _cpcli_path_manager_hpp_

#include <filesystem>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

enum class PathManagerStatus {
  Success,
  RequiredPathNotFound,
  RootPathDoesNotExist,
  RequiredPathDoesNotExist,
  OptionalPathDoesNotExist,
  PathIsNotDirectory,
  FieldDoesNotExist,
};

static inline const std::vector<std::string> REQUIRED_DIR = {"task", "archive", "output", "cpcli"};
static inline const std::vector<std::string> OPTIONAL_DIR = {"include", "template"};

class PathManager {
  using json = nlohmann::json;

public:
  PathManagerStatus path_manager_init(json project_config);

  bool has_customize_template_dir();

  bool has_customize_include_dir();

  std::filesystem::path get(std::string key);

private:
  std::map<std::string, std::filesystem::path> path_mp;
};

#endif
