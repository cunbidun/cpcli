#include <template_manager.hpp>

#include "spdlog/spdlog.h"


TemplateManager::TemplateManager(PathManager &path_manager, std::string language) {
  TemplateManager::has_customized_template_dir = false;
  if (path_manager.has_customize_template_dir()) {
    TemplateManager::has_customized_template_dir = true;
    TemplateManager::customized_path = path_manager.get_template();
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

std::filesystem::path TemplateManager::get_solution() { return TemplateManager::get_path("solution"); };
std::filesystem::path TemplateManager::get_slow() { return TemplateManager::get_path("slow"); };
std::filesystem::path TemplateManager::get_gen() { return TemplateManager::get_path("gen"); };
std::filesystem::path TemplateManager::problem_config() { return TemplateManager::get_path("problem_config"); };
std::filesystem::path TemplateManager::get_checker() { return TemplateManager::get_path("checker"); };
std::filesystem::path TemplateManager::get_interactor() { return TemplateManager::get_path("interactor"); };
