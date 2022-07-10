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

TEST(TestPathManager, TestInitConfigRootOnlySuccess) {
  json project_config;

  auto root = fs::temp_directory_path() / "cpcli_test" / "path_manager";
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
  EXPECT_EQ(path_manager_init(project_config), PathManagerStatus::Success);
  fs::remove_all(root);
}

TEST(TestPathManager, TestInitConfigRootNotExist) {
  json project_config;

  auto root = fs::temp_directory_path() / "cpcli_test" / "path_manager" / "some_dummy_dir";

  project_config["root"] = root;
  EXPECT_EQ(path_manager_init(project_config), PathManagerStatus::RootDirNotExist);
  fs::remove_all(root);
}
