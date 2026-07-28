#include "compiler.hpp"
#include "utils.hpp"
#include <gtest/gtest.h>

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

using json = nlohmann::json;

TEST(TestCompiler, CppCompilationUsesCacheableObjectStep) {
  auto original_dir = std::filesystem::current_path();
  auto test_dir = std::filesystem::temp_directory_path() / gen_string_length_20();
  auto compiler_path = test_dir / "fake-compiler";
  auto log_path = test_dir / "compiler.log";
  auto source_path = test_dir / "slow.cpp";

  std::filesystem::create_directories(test_dir);
  std::ofstream(source_path) << "int main() { return 0; }\n";
  std::ofstream(compiler_path) << R"(#!/bin/sh
printf '%s\n' "$*" >> "$CPCLI_TEST_COMPILER_LOG"
output=
while [ "$#" -gt 0 ]; do
  if [ "$1" = "-o" ]; then
    shift
    output="$1"
  fi
  shift
done
touch "$output"
)";
  std::filesystem::permissions(compiler_path,
                               std::filesystem::perms::owner_read | std::filesystem::perms::owner_write |
                                   std::filesystem::perms::owner_exec);
  setenv("CPCLI_TEST_COMPILER_LOG", log_path.c_str(), 1);

  json project_config = {
      {"language_config",
       {{"[cpp]", {{"compiler", compiler_path.string()}, {"regular_flag", "-DTEST"}, {"debug_flag", "-DDEBUG"}}}}}};
  PathManager path_manager;
  Compiler compiler(project_config, path_manager, false);
  int status = compiler.compile_cpp(source_path, false);
  std::filesystem::current_path(original_dir);

  std::ifstream log(log_path);
  std::vector<std::string> commands;
  for (std::string line; std::getline(log, line);) {
    commands.push_back(line);
  }

  EXPECT_EQ(status, 0);
  ASSERT_EQ(commands.size(), 2);
  EXPECT_NE(commands[0].find("-c"), std::string::npos);
  EXPECT_NE(commands[0].find(".slow.cpcli.o"), std::string::npos);
  EXPECT_NE(commands[1].find(".slow.cpcli.o"), std::string::npos);
  EXPECT_EQ(commands[1].find("-c"), std::string::npos);
  EXPECT_TRUE(std::filesystem::exists(test_dir / "slow"));
  EXPECT_FALSE(std::filesystem::exists(test_dir / ".slow.cpcli.o"));

  unsetenv("CPCLI_TEST_COMPILER_LOG");
  std::filesystem::remove_all(test_dir);
}
