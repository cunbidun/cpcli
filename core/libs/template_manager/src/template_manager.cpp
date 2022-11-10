#include <template_manager.hpp>

#include "spdlog/spdlog.h"

TemplateManager::TemplateManager(PathManager &path_manager, std::string language) {
  TemplateManager::has_customized_template_dir = false;
  if (path_manager.has_customize_template_dir()) {
    TemplateManager::has_customized_template_dir = true;
    TemplateManager::customized_path = path_manager.get_template();
  }
  TemplateManager::builtin_path = path_manager.get_cpcli() / "template" / language;
}

std::optional<std::filesystem::path> TemplateManager::get(std::string str) {
  std::string filename = str + ".template";
  std::filesystem::path path;
  if (TemplateManager::has_customized_template_dir) {
    if (std::filesystem::exists(TemplateManager::customized_path / filename)) {
      path = TemplateManager::customized_path / filename;
    }
  }

  if (path.empty()) {
    if (std::filesystem::exists(TemplateManager::builtin_path / filename)) {
      path = TemplateManager::builtin_path / filename;
    } else {
      spdlog::error("No builtin template found for {}", str);
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
std::filesystem::path TemplateManager::get_config() { return TemplateManager::get_path("config"); };
std::filesystem::path TemplateManager::get_checker() { return TemplateManager::get_path("checker"); };
std::filesystem::path TemplateManager::get_interactor() { return TemplateManager::get_path("interactor"); };
