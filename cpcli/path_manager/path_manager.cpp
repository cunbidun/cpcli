#include "path_manager.hpp"
#include "constant.hpp"
#include "spdlog/spdlog.h"
#include <filesystem>
#include <iostream>

#include <map>

PathManagerStatus PathManager::path_manager_init(json project_config) {
  std::filesystem::path root_path;

  // If the project config contains "root" attribute, infer all other path from the root.
  if (project_config.contains("root")) {
    root_path = std::filesystem::canonical(project_config["root"].get<std::string>());
    if (!std::filesystem::exists(root_path)) {
      spdlog::error("The root_path={} does not exist.", root_path.c_str());
      return PathManagerStatus::RootPathDoesNotExist;
    }
  }

  for (std::string str : REQUIRED_DIR) {
    std::filesystem::path path;
    if (!root_path.empty()) {
      path = root_path / str;
    }
    if (project_config.contains(str)) {
      path = std::filesystem::canonical(project_config[str].get<std::string>());
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
      path = std::filesystem::canonical(project_config[str].get<std::string>());
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
  spdlog::info("Path manager contents:");
  for (auto [k, v] : PathManager::path_mp) {
    spdlog::debug("key={}, value={}", k, v.c_str());
  }

  return PathManagerStatus::Success;
}

bool PathManager::has_customize_template_dir() {
  return PathManager::path_mp.find("template") != PathManager::path_mp.end();
}

bool PathManager::has_customize_include_dir() {
  return PathManager::path_mp.find("include") != PathManager::path_mp.end();
}

std::filesystem::path PathManager::get(string str) {
  if (path_mp.find(str) == path_mp.end()) {
    spdlog::error("key '{}' does not exists in path_mp", str);
  }
  return PathManager::path_mp[str];
}
