#include "path_manager.hpp"
#include "constant.hpp"
#include "glob.hpp"
#include "spdlog/spdlog.h"
#include <filesystem>
#include <iostream>

#include <map>

PathManagerStatus PathManager::init(json project_config) {
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

  return PathManagerStatus::Success;
}

bool PathManager::has_customize_template_dir() {
  return PathManager::path_mp.find("template") != PathManager::path_mp.end();
}

bool PathManager::has_customize_include_dir() {
  return PathManager::path_mp.find("include") != PathManager::path_mp.end();
}

std::filesystem::path PathManager::get_task() { return PathManager::path_mp["task"]; }
std::filesystem::path PathManager::get_output() { return PathManager::path_mp["output"]; }
std::filesystem::path PathManager::get_archive() { return PathManager::path_mp["archive"]; }

std::filesystem::path PathManager::get_local_share() {
  auto home_path = std::filesystem::path(std::getenv("HOME"));
  auto cpcli_data_dir = home_path / ".local" / "share" / "cpcli";
  if (!std::filesystem::exists(cpcli_data_dir) || !std::filesystem::is_directory(cpcli_data_dir)) {
    spdlog::error("the artifacts directory '{}' does not exists or is not a directory", cpcli_data_dir.c_str());
    exit(ArtifactsDirDoesNotExist);
  }
  return cpcli_data_dir;
}

std::filesystem::path PathManager::get_template() { return PathManager::get("template"); }
std::filesystem::path PathManager::get_include() { return PathManager::get("include"); }
std::filesystem::path PathManager::get(std::string str) {
  if (path_mp.find(str) == path_mp.end()) {
    spdlog::error("key '{}' does not exists in path_mp", str);
    exit(PathManagerKeyNotFound);
  }
  return PathManager::path_mp[str];
}

std::filesystem::path PathManager::get_cache_dir(std::filesystem::path root_dir) {
  auto cache_dir =
      std::filesystem::temp_directory_path() / "cpcli" / std::to_string(std::hash<std::string>()(root_dir));
  std::filesystem::create_directories(cache_dir);
  return cache_dir;
}

std::vector<std::filesystem::path> PathManager::get_all_task_path_filetype(std::filesystem::path root_dir,
                                                                           std::string filetype) {
  spdlog::debug("Getting all task files for root_dir={}, filetype={}", root_dir.c_str(), filetype.c_str());
  std::vector<std::filesystem::path> to_return;
  for (auto p : glob::glob((root_dir / filetype).generic_string() + ".*")) {
    if (SUPPORTED_EXTENSIONS.find(p.extension()) != SUPPORTED_EXTENSIONS.end()) {
      to_return.push_back(p);
    }
  }
  filetype[0] = toupper(filetype[0]);
  for (auto p : glob::glob((root_dir / filetype).generic_string() + ".*")) {
    if (SUPPORTED_EXTENSIONS.find(p.extension()) != SUPPORTED_EXTENSIONS.end()) {
      to_return.push_back(p);
    }
  }
  return to_return;
}

bool PathManager::check_task_path_exist(std::filesystem::path root_dir, std::string filetype) {
  spdlog::debug("Checking task file for root_dir={}, filetype={}", root_dir.c_str(), filetype.c_str());
  return PathManager::get_all_task_path_filetype(root_dir, filetype).size() > 0;
}

std::filesystem::path PathManager::get_task_path(std::filesystem::path root_dir, std::string filetype) {
  spdlog::debug("Getting task file for root_dir={}, filetype={}", root_dir.c_str(), filetype.c_str());
  std::vector<std::filesystem::path> path_list = PathManager::get_all_task_path_filetype(root_dir, filetype);
  if (path_list.size() == 1) {
    return path_list[0];
  }
  if (path_list.size() == 0) {
    spdlog::error("No task file found for root_dir={}, filetype={}", root_dir.c_str(), filetype.c_str());
    exit(PathManagerTaskFileNotFound);
  }
  // If there are multiple files, we check if the default extension are one of them.
  // If so, we return that one.
  auto language_config = PathManager::project_config["language_config"];
  auto default_ex = language_config["default"].get<std::string>();
  if (language_config["override"].contains(filetype)) {
    default_ex = language_config["override"][filetype].get<std::string>();
  }
  spdlog::debug("default extension is '.{}'", default_ex);
  for (auto &p : path_list) {
    if (p.extension() == "." + default_ex) {
      return p;
    }
  }
  spdlog::error("Multiple task files found but no match extension for root_dir={}, filetype={}",
                root_dir.c_str(),
                filetype.c_str());
  exit(PathManagerMultipleTaskFilesFound);
}

std::filesystem::path PathManager::get_task_solution_path(std::filesystem::path root_dir) {
  return PathManager::get_task_path(root_dir, "solution");
}
std::filesystem::path PathManager::get_task_slow_path(std::filesystem::path root_dir) {
  return PathManager::get_task_path(root_dir, "slow");
}
std::filesystem::path PathManager::get_task_gen_path(std::filesystem::path root_dir) {
  return PathManager::get_task_path(root_dir, "gen");
}
std::filesystem::path PathManager::get_task_checker_path(std::filesystem::path root_dir) {
  return PathManager::get_task_path(root_dir, "checker");
}
std::filesystem::path PathManager::get_task_interactor_path(std::filesystem::path root_dir) {
  return PathManager::get_task_path(root_dir, "interactor");
}
