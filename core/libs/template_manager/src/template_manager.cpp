#include "template_manager.hpp"
#include "inja.hpp"
#include "spdlog/spdlog.h"

#include <fstream>
#include <iostream>

TemplateManager::TemplateManager(PathManager &path_manager, std::string language, bool use_template_engine) {
  TemplateManager::has_customized_template_dir = false;
  TemplateManager::use_template_engine = use_template_engine;
  if (path_manager.has_customize_template_dir()) {
    TemplateManager::has_customized_template_dir = true;
    TemplateManager::customized_path = path_manager.get_template();
  }
  if (TemplateManager::use_template_engine && !TemplateManager::has_customized_template_dir) {
    spdlog::error("Template engine is enabled but no customized template directory is found. Please create and "
                  "populate 'data.json' file in your template folder");
    exit(TemplateManagerTemplateEngineError);
  }
  if (TemplateManager::use_template_engine) {
    if (!std::filesystem::exists(TemplateManager::customized_path / "data.json")) {
      spdlog::error(
          "Template engine is enabled. There is a customized template dir but no 'data.json' found. Please create and "
          "populate 'data.json' file in your template folder");
      exit(TemplateManagerTemplateEngineError);
    }
  }

  TemplateManager::builtin_language_template_dir = path_manager.get_local_share() / "templates" / language;
  TemplateManager::builtin_common_template_dir = path_manager.get_local_share() / "templates" / "common";
}
/**
 * @brief A artifact directory will look like this:
 *
 * ├── checkers
 * │   ├── double_4
 * │   ├── double_6
 * │   ├── double_9
 * │   └── token_checker
 * ├── frontend
 * │   └── TaskConfigEditor.jar
 * └── templates
 *    ├── common
 *    │   ├── problem_config.template
 *    │   └── project_config.template
 *    └── cpp
 *        ├── checker.template
 *        ├── gen.template
 *        ├── interactor.template
 *        ├── slow.template
 *        └── solution.template
 *
 * @param str
 * @return std::optional<std::filesystem::path>
 */
std::optional<std::filesystem::path> TemplateManager::get(std::string str) {
  std::string filename = str + ".template";
  std::filesystem::path path;
  if (TemplateManager::has_customized_template_dir) {
    if (std::filesystem::exists(TemplateManager::customized_path / filename)) {
      path = TemplateManager::customized_path / filename;
    }
  }

  if (path.empty()) {
    std::filesystem::path builtin_path = TemplateManager::builtin_language_template_dir;
    if (str == "problem_config") {
      builtin_path = TemplateManager::builtin_common_template_dir;
    }
    spdlog::debug("The build in template path is builtin_path={}", builtin_path.c_str());
    if (std::filesystem::exists(builtin_path / filename)) {
      path = builtin_path / filename;
    } else {
      spdlog::error("No builtin template found for {} ({})", str, (builtin_path / filename).c_str());
      return std::nullopt;
    }
  }
  if (!path.empty() && std::filesystem::status(path).type() != std::filesystem::file_type::regular) {
    spdlog::error("Template file for {} found, but it is not a regular file", str);
    return std::nullopt;
  }
  return path;
}

std::filesystem::path TemplateManager::get_path(std::string str) {
  auto path = TemplateManager::get(str);
  if (path) {
    return *path;
  }
  spdlog::error("Required template for {} not found", str);
  exit(TemplateManagerRequiredTemplateNotFound);
};

void TemplateManager::render(std::filesystem::path template_file, std::filesystem::path location) {
  if (use_template_engine) {
    spdlog::debug(
        "Rendering template {} to {} with use_template_engine enable", template_file.c_str(), location.c_str());
    inja::Environment env;
    inja::Template template_obj = env.parse_template(template_file);
    std::ifstream ifs(TemplateManager::customized_path / "data.json");
    auto data = json::parse(ifs);

    // compute current time and add it to data
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm local_time;
    localtime_r(&in_time_t, &local_time);
    std::ostringstream oss;
    oss << std::put_time(&local_time, data.value("time_format", "%A, %Y-%m-%d %H:%M:%S %Z").c_str());
    // TODO add docs on template engine
    data["__now__"] = oss.str();

    env.write(template_obj, data, location);
  } else {
    spdlog::debug(
        "Copying template {} to {} with use_template_engine disable", template_file.c_str(), location.c_str());
    std::filesystem::copy_file(template_file, location, std::filesystem::copy_options::overwrite_existing);
  }
}

std::filesystem::path TemplateManager::get_solution() { return TemplateManager::get_path("solution"); };
std::filesystem::path TemplateManager::get_slow() { return TemplateManager::get_path("slow"); };
std::filesystem::path TemplateManager::get_gen() { return TemplateManager::get_path("gen"); };
std::filesystem::path TemplateManager::get_problem_config() { return TemplateManager::get_path("problem_config"); };
std::filesystem::path TemplateManager::get_checker() { return TemplateManager::get_path("checker"); };
std::filesystem::path TemplateManager::get_interactor() { return TemplateManager::get_path("interactor"); };
