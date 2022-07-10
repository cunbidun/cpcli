#ifndef _cpcli_path_manager_hpp_
#define _cpcli_path_manager_hpp_

#include <filesystem>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

enum class PathManagerStatus { Success, RequiredDirNotFound, DirNotExist, RootDirNotExist, CustomPathDoesNotExist };

static inline const std::vector<std::string> REQUIRED_DIR = {"task", "archive", "output", "cpcli"};
static inline const std::vector<std::string> OPTIONAL_DIR = {"include", "template"};

using json = nlohmann::json;

PathManagerStatus path_manager_init(json project_config);
#endif
