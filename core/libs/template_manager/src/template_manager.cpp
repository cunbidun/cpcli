#include "template_manager.hpp"
#include "inja.hpp"
#include "spdlog/spdlog.h"

#include <fstream>
#include <iostream>

TemplateManager::TemplateManager(PathManager &path_manager, json project_config) {
  TemplateManager::has_customized_template_dir = false;
  TemplateManager::use_template_engine = project_config.value("use_template_engine", false);
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
  TemplateManager::builtin_common_template_dir = path_manager.get_local_share() / "templates" / "common";
  TemplateManager::project_config = project_config;
  TemplateManager::path_manager = path_manager;
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
 *     ├── common
 *     │   ├── problem_config.template
 *     │   └── project_config.template
 *     ├── cpp
 *     │   ├── checker.template
 *     │   ├── gen.template
 *     │   ├── interactor.template
 *     │   ├── slow.template
 *     │   └── solution.template
 *     └── python
 *         ├── checker.template
 *         ├── gen.template
 *         ├── interactor.template
 *         ├── slow.template
 *         └── solution.template
 *
 * @param str
 * @return std::optional<std::filesystem::path>
 */
std::optional<std::filesystem::path> TemplateManager::get(std::string str) {
  spdlog::debug("Get the template file for {}", str);
  std::string filename = str + ".template";
  std::filesystem::path path;
  std::string language = project_config["language_config"]["default"].get<std::string>();
  if (project_config["language_config"]["override"].contains(str)) {
    language = project_config["language_config"]["override"][str].get<std::string>();
  }
  spdlog::debug("The language for {} is {}", str, language);
  if (TemplateManager::has_customized_template_dir) {
    if (std::filesystem::exists(TemplateManager::customized_path / language / filename)) {
      path = TemplateManager::customized_path / language / filename;
    }
  }

  if (path.empty()) {
    // At this point, the path is empty when the customized template dir is not used or
    // the file is not found in the customized template dir.
    // In both cases, we should try to find the file in the builtin template dir.

    // If the requested file is project_config.template, we will find it in common template dir
    // Otherwise, we will find it in the language specific template dir.

    auto builtin_path = TemplateManager::path_manager.get_local_share() / "templates" / language;
    if (str == "problem_config") {
      builtin_path = TemplateManager::builtin_common_template_dir;
    }
    spdlog::debug("The build in template path is builtin_path={}", builtin_path.c_str());

    // Check if the template file exists
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

void TemplateManager::render(std::filesystem::path template_file, std::filesystem::path location, bool overwrite) {
  if (std::filesystem::status(location).type() != std::filesystem::file_type::directory) {
    spdlog::error("The location {} is not a directory ", location.c_str());
    exit(1);
  }
  spdlog::debug("Render the template file {} to directory {}", template_file.c_str(), location.c_str());
  std::string filetype = template_file.stem();
  std::string filename;
  if (filetype == "config") {
    filename = "config.json";
  } else {
    std::string extension = template_file.parent_path().filename();
    filename = filetype + "." + extension;
  }
  location = location / filename;
  spdlog::debug("File", template_file.c_str(), location.c_str());

  if (!overwrite && std::filesystem::exists(location)) {
    spdlog::debug("File {} already exists, skip rendering", location.c_str());
    return;
  }
  if (use_template_engine) {
    spdlog::debug(
        "Rendering template {} to {} with use_template_engine enable", template_file.c_str(), location.c_str());
    inja::Environment env;
    inja::Template template_obj = env.parse_template(template_file);
    std::ifstream ifs(TemplateManager::customized_path / "data.json");
    auto data = json::parse(ifs);

    // Compute current time and add it to data
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm local_time;
    localtime_r(&in_time_t, &local_time);
    std::ostringstream oss;
    oss << std::put_time(&local_time, data.value("time_format", DEFAULT_TEMPLATE_DATETIME_FORMAT).c_str());
    // TODO add docs on template engine
    data["__now__"] = oss.str();

    // Write the rendered template to file
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
