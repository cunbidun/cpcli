#ifndef _cpcli_template_manager_hpp_
#define _cpcli_template_manager_hpp_

#include "path_manager.hpp"
#include <filesystem>
#include <nlohmann/json.hpp>
#include <optional>

static int const TemplateManagerRequiredTemplateNotFound = 168372;
static int const TemplateManagerTemplateEngineError = 168375;
static const std::string DEFAULT_TEMPLATE_DATETIME_FORMAT = "%A, %Y-%m-%d %H:%M:%S %Z";

class TemplateManager {
  using json = nlohmann::json;

public:
  TemplateManager(PathManager &path_manager, std::string language, bool use_template_engine);
  std::filesystem::path get_solution();
  std::filesystem::path get_slow();
  std::filesystem::path get_gen();
  std::filesystem::path get_problem_config();
  std::filesystem::path get_checker();
  std::filesystem::path get_interactor();
  void render(std::filesystem::path template_file, std::filesystem::path location, bool overwrite);

private:
  bool has_customized_template_dir;
  bool use_template_engine;
  std::filesystem::path builtin_language_template_dir;
  std::filesystem::path builtin_common_template_dir;
  std::filesystem::path customized_path;
  std::optional<std::filesystem::path> get(std::string str);
  std::filesystem::path get_path(std::string str);
};

#endif
