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
  auto test_dir = fs::temp_directory_path() / str;
  auto root = test_dir / "cpcli_test" / "path_manager";
  fs::create_directories(root);

  auto task = root / "task";
  auto archive = root / "archive";
  auto output = root / "output";
  auto cpcli = root / "cpcli";
  fs::create_directory(task);
  fs::create_directory(archive);
  fs::create_directory(output);
  fs::create_directory(cpcli);

  project_config["root"] = root;
  PathManager manager;
  EXPECT_EQ(manager.init(project_config), PathManagerStatus::Success);
  EXPECT_EQ(manager.has_customize_include_dir(), false);
  EXPECT_EQ(manager.has_customize_template_dir(), false);

  fs::remove_all(test_dir);
}

TEST(TestPathManager, TestInitConfigRootOnlySuccessAll) {
  json project_config;

  auto str = gen_string_length_20();
  auto test_dir = fs::temp_directory_path() / str;
  auto root = test_dir / "cpcli_test" / "path_manager";
  fs::create_directories(root);

  auto task = root / "task";
  auto archive = root / "archive";
  auto output = root / "output";
  auto cpcli = root / "cpcli";
  auto include_dir = root / "include";
  auto template_dir = root / "template";
  fs::create_directory(task);
  fs::create_directory(archive);
  fs::create_directory(output);
  fs::create_directory(cpcli);
  fs::create_directory(include_dir);
  fs::create_directory(template_dir);
  project_config["root"] = root;

  PathManager manager;
  EXPECT_EQ(manager.init(project_config), PathManagerStatus::Success);
  EXPECT_EQ(manager.has_customize_include_dir(), true);
  EXPECT_EQ(manager.has_customize_template_dir(), true);
  fs::remove_all(test_dir);
}

TEST(TestPathManager, TestInitConfigRootOnlyIncludeOnly) {
  json project_config;

  auto str = gen_string_length_20();
  auto test_dir = fs::temp_directory_path() / str;
  auto root = test_dir / "cpcli_test" / "path_manager";
  fs::create_directories(root);

  auto task = root / "task";
  auto archive = root / "archive";
  auto output = root / "output";
  auto cpcli = root / "cpcli";
  auto include_dir = root / "include";
  fs::create_directory(task);
  fs::create_directory(archive);
  fs::create_directory(output);
  fs::create_directory(cpcli);
  fs::create_directory(include_dir);
  project_config["root"] = root;

  PathManager manager;
  EXPECT_EQ(manager.init(project_config), PathManagerStatus::Success);
  EXPECT_EQ(manager.has_customize_include_dir(), true);
  EXPECT_EQ(manager.has_customize_template_dir(), false);
  fs::remove_all(test_dir);
}

TEST(TestPathManager, TestInitConfigRootNotExist) {
  json project_config;
  auto str = gen_string_length_20();
  auto test_dir = fs::temp_directory_path() / str;
  auto root = test_dir / "cpcli_test" / "path_manager";
  project_config["root"] = root;
  PathManager manager;
  EXPECT_EQ(manager.init(project_config), PathManagerStatus::RootPathDoesNotExist);
  fs::remove_all(test_dir);
}
