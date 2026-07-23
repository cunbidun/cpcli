#include "path_manager.hpp"
#include "utils.hpp"
#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

using json = nlohmann::json;

using std::string;

TEST(TestPathManager, TestInitConfigRootOnlySuccessRequired) {
  json project_config;

  auto str = gen_string_length_20();
  auto test_dir = std::filesystem::temp_directory_path() / str;
  auto root = test_dir / "cpcli_test" / "path_manager";
  std::filesystem::create_directories(root);

  auto task = root / "task";
  auto archive = root / "archive";
  auto output = root / "output";
  auto cpcli = root / "cpcli";
  std::filesystem::create_directory(task);
  std::filesystem::create_directory(archive);
  std::filesystem::create_directory(output);
  std::filesystem::create_directory(cpcli);

  project_config["root"] = root;
  PathManager manager;
  EXPECT_EQ(manager.init(project_config), PathManagerStatus::Success);
  EXPECT_EQ(manager.has_customize_template_dir(), false);

  std::filesystem::remove_all(test_dir);
}

TEST(TestPathManager, TestInitConfigRootOnlySuccessAll) {
  json project_config;

  auto str = gen_string_length_20();
  auto test_dir = std::filesystem::temp_directory_path() / str;
  auto root = test_dir / "cpcli_test" / "path_manager";
  std::filesystem::create_directories(root);

  auto task = root / "task";
  auto archive = root / "archive";
  auto output = root / "output";
  auto cpcli = root / "cpcli";
  auto include_dir = root / "include";
  auto template_dir = root / "template";
  std::filesystem::create_directory(task);
  std::filesystem::create_directory(archive);
  std::filesystem::create_directory(output);
  std::filesystem::create_directory(cpcli);
  std::filesystem::create_directory(template_dir);
  project_config["root"] = root;

  PathManager manager;
  EXPECT_EQ(manager.init(project_config), PathManagerStatus::Success);
  EXPECT_EQ(manager.has_customize_template_dir(), true);
  std::filesystem::remove_all(test_dir);
}

TEST(TestPathManager, TestInitConfigRootOnlyIncludeOnly) {
  json project_config;

  auto str = gen_string_length_20();
  auto test_dir = std::filesystem::temp_directory_path() / str;
  auto root = test_dir / "cpcli_test" / "path_manager";
  std::filesystem::create_directories(root);

  auto task = root / "task";
  auto archive = root / "archive";
  auto output = root / "output";
  auto cpcli = root / "cpcli";
  auto include_dir = root / "include";
  std::filesystem::create_directory(task);
  std::filesystem::create_directory(archive);
  std::filesystem::create_directory(output);
  std::filesystem::create_directory(cpcli);
  std::filesystem::create_directory(include_dir);
  project_config["root"] = root;

  PathManager manager;
  EXPECT_EQ(manager.init(project_config), PathManagerStatus::Success);
  EXPECT_EQ(manager.has_customize_template_dir(), false);
  std::filesystem::remove_all(test_dir);
}

TEST(TestPathManager, SelectsCudaSolutionFromProblemOverride) {
  auto test_dir = std::filesystem::temp_directory_path() / gen_string_length_20();
  auto root = test_dir / "cpcli_test" / "path_manager";
  auto task_dir = root / "task" / "cuda_task";
  std::filesystem::create_directories(task_dir);
  std::filesystem::create_directory(root / "archive");
  std::filesystem::create_directory(root / "output");

  std::ofstream(task_dir / "solution.cpp") << "int main() {}\n";
  std::ofstream(task_dir / "solution.cu") << "__global__ void kernel() {}\n";

  json project_config = {
      {"root", root}, {"language_config", {{"default", "cpp"}, {"override", json::object()}}}};
  json problem_config = {{"languageConfig", {{"solution", "cu"}}}};

  PathManager manager;
  ASSERT_EQ(manager.init(project_config, problem_config), PathManagerStatus::Success);
  EXPECT_EQ(manager.get_solution_path(task_dir), std::filesystem::canonical(task_dir / "solution.cu"));

  std::filesystem::remove_all(test_dir);
}

TEST(TestPathManager, FindsSupportedMultiDotTaskFile) {
  auto test_dir = std::filesystem::temp_directory_path() / gen_string_length_20();
  auto task_dir = test_dir / "task";
  std::filesystem::create_directories(task_dir);
  std::filesystem::create_directory(test_dir / "archive");
  std::filesystem::create_directory(test_dir / "output");
  auto solution_path = task_dir / "solution.reference.cpp";
  std::ofstream(solution_path) << "int main() {}\n";

  json project_config = {
      {"root", test_dir}, {"language_config", {{"default", "cpp"}, {"override", json::object()}}}};
  PathManager manager;
  ASSERT_EQ(manager.init(project_config), PathManagerStatus::Success);

  EXPECT_EQ(manager.get_solution_path(task_dir), solution_path);
  std::filesystem::remove_all(test_dir);
}

TEST(TestPathManager, TestInitConfigRootNotExist) {
  json project_config;
  auto str = gen_string_length_20();
  auto test_dir = std::filesystem::temp_directory_path() / str;
  auto root = test_dir / "cpcli_test" / "path_manager";
  project_config["root"] = root;
  PathManager manager;
  EXPECT_EQ(manager.init(project_config), PathManagerStatus::RootPathDoesNotExist);
  std::filesystem::remove_all(test_dir);
}
