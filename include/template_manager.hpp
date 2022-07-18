#ifndef _cpcli_tempalte_manager_hpp_
#define _cpcli_tempalte_manager_hpp_

#include "path_manager.hpp"
#include <filesystem>
#include <nlohmann/json.hpp>

static int const TemplateManagerRequiredTemplateNotFound = 168372;

class TemplateManager {
  using json = nlohmann::json;

public:
  TemplateManager(PathManager &path_manager, std::string language);
  std::filesystem::path get_solution();
  std::filesystem::path get_slow();
  std::filesystem::path get_gen();
  std::filesystem::path get_config();
  std::filesystem::path get_checker();
  std::filesystem::path get_interactor();

private:
  bool has_customize_tempate_dir;
  std::filesystem::path builtin_path;
  std::filesystem::path cusomize_path;
  std::optional<std::filesystem::path> get(std::string str);
  std::filesystem::path get_path(std::string str);
};

#endif
