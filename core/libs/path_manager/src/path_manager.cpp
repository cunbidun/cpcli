#include "path_manager.hpp"
#include "constant.hpp"
#include "spdlog/spdlog.h"
#include <cctype>
#include <cstdlib>
#include <filesystem>
#include <iostream>

#include <map>
#if defined(__APPLE__)
#include <mach-o/dyld.h>
#endif

namespace {
std::filesystem::path find_installed_data_dir() {
  std::filesystem::path executable;
#if defined(__linux__)
  std::error_code error;
  executable = std::filesystem::read_symlink("/proc/self/exe", error);
  if (error) {
    return {};
  }
#elif defined(__APPLE__)
  uint32_t size = 0;
  _NSGetExecutablePath(nullptr, &size);
  std::vector<char> buffer(size);
  if (_NSGetExecutablePath(buffer.data(), &size) != 0) {
    return {};
  }
  executable = std::filesystem::weakly_canonical(buffer.data());
#else
  return {};
#endif
  auto directory = executable.parent_path();
  for (int i = 0; i < 4 && !directory.empty(); ++i) {
    if (std::filesystem::is_directory(directory / "templates") &&
        std::filesystem::is_directory(directory / "checkers")) {
      return directory;
    }
    auto data_dir = directory / "share" / "cpcli";
    if (std::filesystem::is_directory(data_dir)) {
      return data_dir;
    }
    directory = directory.parent_path();
  }
  return {};
}
} // namespace

PathManagerStatus PathManager::init(json project_config, json problem_config) {
  std::filesystem::path root_path;

  // If the project config contains "root" attribute, infer all other path from the root.
  if (project_config.contains("root")) {
    root_path = project_config["root"].get<std::string>();
    if (!std::filesystem::exists(root_path)) {
      spdlog::error("The root_path={} does not exist.", root_path.c_str());
      return PathManagerStatus::RootPathDoesNotExist;
    }
    root_path = std::filesystem::canonical(root_path);
  }

  for (std::string str : REQUIRED_DIR) {
    std::filesystem::path path;
    if (!root_path.empty()) {
      path = root_path / str;
    }
    if (project_config.contains(str)) {
      path = project_config[str].get<std::string>();
    }
    if (path.empty()) {
      spdlog::error("The required path for '{}' not found", str);
      return PathManagerStatus::RequiredPathNotFound;
    }
    if (!std::filesystem::exists(path)) {
      spdlog::error("The required path for '{}' found, but does not exists", str);
      return PathManagerStatus::RequiredPathDoesNotExist;
    }
    if (std::filesystem::status(path).type() != std::filesystem::file_type::directory) {
      return PathManagerStatus::PathIsNotDirectory;
    }
    PathManager::path_mp[str] = path;
  }

  for (std::string str : OPTIONAL_DIR) {
    std::filesystem::path path;
    if (!root_path.empty()) {
      path = root_path / str;
    }
    if (project_config.contains(str)) {
      path = project_config[str].get<std::string>();
      if (!std::filesystem::exists(path)) {
        spdlog::error("The optional path for '{}' found, but does not exists", str);
        return PathManagerStatus::OptionalPathDoesNotExist;
      }
    }
    if (!path.empty() && std::filesystem::exists(path)) {
      if (std::filesystem::status(path).type() != std::filesystem::file_type::directory) {
        return PathManagerStatus::PathIsNotDirectory;
      }
      PathManager::path_mp[str] = path;
    } else {
      spdlog::debug("path={} not found, ignoring", path.c_str());
    }
  }
  spdlog::debug("Path manager contents:");
  for (auto [k, v] : PathManager::path_mp) {
    PathManager::path_mp[k] = std::filesystem::canonical(v);
    spdlog::debug("key={}, value={}", k, v.c_str());
  }
  PathManager::project_config = project_config;
  PathManager::problem_config = problem_config;

  return PathManagerStatus::Success;
}

void PathManager::init_problem_conf(json problem_config) { PathManager::problem_config = problem_config; }

bool PathManager::has_customize_template_dir() {
  return PathManager::path_mp.find("template") != PathManager::path_mp.end();
}

std::filesystem::path PathManager::get_task() { return PathManager::path_mp["task"]; }
std::filesystem::path PathManager::get_output() { return PathManager::path_mp["output"]; }
std::filesystem::path PathManager::get_archive() { return PathManager::path_mp["archive"]; }

std::filesystem::path PathManager::get_local_share() {
  if (const char *data_dir = std::getenv("CPCLI_DATA_DIR"); data_dir != nullptr && data_dir[0] != '\0') {
    auto configured_dir = std::filesystem::path(data_dir);
    if (std::filesystem::is_directory(configured_dir)) {
      return configured_dir;
    }
    spdlog::error("CPCLI_DATA_DIR '{}' does not exist or is not a directory", configured_dir.c_str());
    exit(ArtifactsDirDoesNotExist);
  }
  if (auto installed_data_dir = find_installed_data_dir(); !installed_data_dir.empty()) {
    return installed_data_dir;
  }
  auto home_path = std::filesystem::path(std::getenv("HOME"));
  auto cpcli_data_dir = home_path / ".local" / "share" / "cpcli";
  if (!std::filesystem::exists(cpcli_data_dir) || !std::filesystem::is_directory(cpcli_data_dir)) {
    spdlog::error("the artifacts directory '{}' does not exists or is not a directory", cpcli_data_dir.c_str());
    exit(ArtifactsDirDoesNotExist);
  }
  return cpcli_data_dir;
}

std::filesystem::path PathManager::get_template() { return PathManager::get("template"); }
std::filesystem::path PathManager::get(std::string str) {
  if (path_mp.find(str) == path_mp.end()) {
    spdlog::error("key '{}' does not exists in path_mp", str);
    exit(PathManagerKeyNotFound);
  }
  return PathManager::path_mp[str];
}

std::vector<std::filesystem::path> PathManager::get_all_task_path_filetype(std::filesystem::path root_dir,
                                                                           std::string filetype) {
  spdlog::debug("Getting all task files for root_dir={}, filetype={}", root_dir.c_str(), filetype.c_str());
  std::vector<std::filesystem::path> to_return;
  std::string capitalized_filetype = filetype;
  capitalized_filetype[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(capitalized_filetype[0])));
  for (const auto &entry : std::filesystem::directory_iterator(root_dir)) {
    if (!entry.is_regular_file()) {
      continue;
    }
    const auto &path = entry.path();
    const auto filename = path.filename().string();
    const bool matches_filetype =
        filename.rfind(filetype + ".", 0) == 0 || filename.rfind(capitalized_filetype + ".", 0) == 0;
    if (matches_filetype && SUPPORTED_EXTENSIONS.find(path.extension()) != SUPPORTED_EXTENSIONS.end()) {
      to_return.push_back(path);
    }
  }
  return to_return;
}

bool PathManager::check_task_path_exist(std::filesystem::path root_dir, std::string filetype) {
  spdlog::debug("Checking task file for root_dir={}, filetype={}", root_dir.c_str(), filetype.c_str());
  return PathManager::get_all_task_path_filetype(root_dir, filetype).size() > 0;
}

std::filesystem::path PathManager::get_full_path_with_file_type(std::filesystem::path root_dir,
                                                                std::string filetype,
                                                                std::string override_key) {
  spdlog::debug("Getting task file for root_dir={}, filetype={}", root_dir.c_str(), filetype.c_str());
  std::vector<std::filesystem::path> path_list = PathManager::get_all_task_path_filetype(root_dir, filetype);
  if (path_list.size() == 0) {
    spdlog::error("No task file found for root_dir={}, filetype={}", root_dir.c_str(), filetype.c_str());
    exit(PathManagerTaskFileNotFound);
  }
  // If there are multiple files, we check if the default extension are one of them.
  // If so, we return that one.
  auto language_config = PathManager::project_config["language_config"];
  auto ex = language_config["default"].get<std::string>();
  bool has_explicit_override = false;
  spdlog::debug("Default extension is {}", ex);

  // project config override
  if (language_config.contains("override")) {
    if (language_config["override"].contains(override_key)) {
      ex = language_config["override"][override_key].get<std::string>();
      has_explicit_override = true;
    }
    spdlog::debug("Project config contains a override map, new extension {}", ex);
  }

  // problem config override
  if (!PathManager::problem_config.empty() && PathManager::problem_config.contains("languageConfig")) {
    language_config = PathManager::problem_config["languageConfig"];
    if (language_config.contains("default") && language_config["default"].is_string()) {
      ex = language_config["default"].get<std::string>();
      has_explicit_override = true;
    }
    if (language_config.contains(override_key) && !language_config[override_key].is_null()) {
      ex = language_config[override_key].get<std::string>();
      has_explicit_override = true;
    }
    spdlog::debug("Problem config contains a override map, new extension is '.{}'", ex);
  }

  spdlog::debug("Final extension is '.{}'", ex);
  for (auto &p : path_list) {
    if (p.extension() == "." + ex) {
      return p;
    }
  }
  if (!has_explicit_override && path_list.size() == 1) {
    return path_list[0];
  }
  spdlog::error("Multiple task files found but no match extension for root_dir={}, filetype={}",
                root_dir.c_str(),
                filetype.c_str());
  exit(PathManagerMultipleTaskFilesFound);
}

std::filesystem::path PathManager::get_solution_path(std::filesystem::path root_dir, std::string name) {
  return PathManager::get_full_path_with_file_type(root_dir, name, "solution");
}
std::filesystem::path PathManager::get_slow_path(std::filesystem::path root_dir, std::string name) {
  return PathManager::get_full_path_with_file_type(root_dir, name, "slow");
}
std::filesystem::path PathManager::get_task_gen_path(std::filesystem::path root_dir, std::string name) {
  return PathManager::get_full_path_with_file_type(root_dir, name, "gen");
}
std::filesystem::path PathManager::get_checker_path(std::filesystem::path root_dir, std::string name) {
  return PathManager::get_full_path_with_file_type(root_dir, name, "checker");
}
std::filesystem::path PathManager::get_interactor_path(std::filesystem::path root_dir, std::string name) {
  return PathManager::get_full_path_with_file_type(root_dir, name, "interactor");
}
