#ifndef _cpcli_path_manager_hpp_
#define _cpcli_path_manager_hpp_

#include <filesystem>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

static int const PathManagerKeyNotFound = 268000;
static int const PathManagerFailToInitFromConfig = 268001;
static int const ArtifactsDirDoesNotExist = 268002;
static int const PathManagerTaskFileNotFound = 268003;
static int const PathManagerMultipleTaskFilesFound = 268004;

enum class PathManagerStatus {
  Success,
  RequiredPathNotFound,
  RootPathDoesNotExist,
  RequiredPathDoesNotExist,
  OptionalPathDoesNotExist,
  PathIsNotDirectory,
  FieldDoesNotExist,
};

static inline const std::vector<std::string> REQUIRED_DIR = {"task", "archive", "output"};
static inline const std::vector<std::string> OPTIONAL_DIR = {"include", "template"};

class PathManager {
  using json = nlohmann::json;

public:
  PathManagerStatus init(json project_config);

  bool has_customize_template_dir();
  bool has_customize_include_dir();

  // Required directories
  /**
   * @brief Get the local share path
   *      This path is used to store the local artifacts share templates and binaries
   *
   * @return std::filesystem::path to $HOME/.local/share/cpcli on Linux and Mac
   */
  std::filesystem::path get_local_share();
  std::filesystem::path get_task();
  std::filesystem::path get_output();
  std::filesystem::path get_archive();

  // Required optional dir
  std::filesystem::path get_template();
  std::filesystem::path get_include();

  std::filesystem::path get_cache_dir(std::filesystem::path path);
  std::filesystem::path get_task_solution_path(std::filesystem::path root_dir);
  std::filesystem::path get_task_slow_path(std::filesystem::path root_dir);
  std::filesystem::path get_task_gen_path(std::filesystem::path root_dir);
  std::filesystem::path get_task_checker_path(std::filesystem::path root_dir);
  std::filesystem::path get_task_interactor_path(std::filesystem::path root_dir);

  bool check_task_path_exist(std::filesystem::path root_dir, std::string filetype);

private:
  json project_config;
  std::map<std::string, std::filesystem::path> path_mp;
  std::filesystem::path get(std::string key);
  std::filesystem::path get_task_path(std::filesystem::path root_dir, std::string filetype);
  std::vector<std::filesystem::path> get_all_task_path_filetype(std::filesystem::path root_dir, std::string filetype);
};

#endif
