#include "path_manager.hpp"
#include "utils.hpp"
#include <gtest/gtest.h>

#include <filesystem>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

using json = nlohmann::json;

namespace fs = std::filesystem;
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
  EXPECT_EQ(manager.has_customize_include_dir(), false);
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
  std::filesystem::create_directory(include_dir);
  std::filesystem::create_directory(template_dir);
  project_config["root"] = root;

  PathManager manager;
  EXPECT_EQ(manager.init(project_config), PathManagerStatus::Success);
  EXPECT_EQ(manager.has_customize_include_dir(), true);
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
  EXPECT_EQ(manager.has_customize_include_dir(), true);
  EXPECT_EQ(manager.has_customize_template_dir(), false);
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
