#include "path_manager.hpp"
#include "constant.hpp"
#include "spdlog/spdlog.h"
#include <filesystem>
#include <iostream>

#include <map>

std::map<std::string, std::filesystem::path> path_mp;

PathManagerStatus path_manager_init(json project_config) {
  auto all_dir = std::vector<std::string>();
  for (auto dir : REQUIRED_DIR) {
    all_dir.push_back(dir);
  }
  for (auto dir : OPTIONAL_DIR) {
    all_dir.push_back(dir);
  }

  if (project_config.contains("root")) {
    std::filesystem::path root_path = project_config["root"].get<std::string>();
    if (!std::filesystem::exists(root_path)) {
      return PathManagerStatus::RootDirNotExist;
    }
    for (std::string str : all_dir) {
      path_mp[str] = root_path / str;
    }
  }
  for (std::string str : all_dir) {
    if (project_config.contains(str)) {
      auto path = project_config[str].get<std::string>();
      if (!std::filesystem::exists(path)) {
        return PathManagerStatus::CustomPathDoesNotExist;
      }
      path_mp[str] = path;
    }
  }
  for (std::string str : REQUIRED_DIR) {
    if (path_mp.find(str) == path_mp.end()) {
      return PathManagerStatus::RequiredDirNotFound;
    }
    auto path = path_mp[str];
    if (!std::filesystem::exists(path)) {
      return PathManagerStatus::RequiredDirNotFound;
    }
  }
  for (std::string str : OPTIONAL_DIR) {
    if (path_mp.find(str) != path_mp.end()) {
      auto path = path_mp[str];
      if (!std::filesystem::exists(path)) {
        path_mp.erase(str);
      }
    }
  }
  return PathManagerStatus::Success;
}

bool has_customize_template_dir() { return path_mp.find("template") != path_mp.end(); }

bool has_customize_include_dir() { return path_mp.find("include") != path_mp.end(); }
