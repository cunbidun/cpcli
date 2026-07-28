#include "operations.hpp"
#include "utils.hpp"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

namespace {
class TemporaryConfig {
public:
  explicit TemporaryConfig(const std::string &contents) {
    directory = std::filesystem::temp_directory_path() / gen_string_length_20();
    std::filesystem::create_directories(directory);
    path = directory / "config.toml";
    std::ofstream(path) << contents;
  }

  ~TemporaryConfig() { std::filesystem::remove_all(directory); }

  std::filesystem::path path;

  std::filesystem::path output_path() const { return directory / "written.toml"; }

private:
  std::filesystem::path directory;
};
} // namespace

TEST(TestParseConfig, LegacyConfigNormalizesToImplicitSubtask) {
  TemporaryConfig config(R"(
[problem]
name = "Legacy"
group = "Tests"

[run]
checker = "double_6"
interactive = false
time_limit_ms = 2500
stop_on_first_failure = false

[generation]
enabled = true
test_count = 7
seed = "fixed"
args = "--small"
expected_output_from_slow = true

[[tests]]
enabled = true
input = "1\n"
output = "1\n"
has_expected_output = true
)");

  const auto parsed = parse_problem_config_file(config.path);

  ASSERT_EQ(parsed["subtasks"].size(), 1);
  const auto &subtask = parsed["subtasks"][0];
  EXPECT_EQ(subtask["index"], 0);
  EXPECT_EQ(subtask["name"], "");
  EXPECT_TRUE(subtask["enabled"]);
  EXPECT_EQ(subtask["checker"], "double_6");
  EXPECT_EQ(subtask["timeLimit"], 2500);
  EXPECT_EQ(subtask["gen"], "gen");
  EXPECT_EQ(subtask["slow"], "slow");
  EXPECT_TRUE(subtask["useGeneration"]);
  EXPECT_EQ(subtask["numTest"], 7);
  EXPECT_EQ(subtask["generatorSeed"], "fixed");
  EXPECT_EQ(subtask["genParameters"], "--small");
  EXPECT_TRUE(subtask["knowGenAns"]);
  EXPECT_EQ(parsed["tests"][0]["subtaskIndex"], 0);
  EXPECT_EQ(parsed["tests"][0]["subtask"], "");
}

TEST(TestParseConfig, ExplicitSubtasksInheritDefaultsAndOwnTests) {
  TemporaryConfig config(R"(
[problem]
name = "Subtasks"
group = "Tests"

[run]
checker = "token_checker"
interactive = false
time_limit_ms = 3000
stop_on_first_failure = false

[generation]
enabled = false
test_count = 0
seed = "root-seed"
args = ""
expected_output_from_slow = false

[[subtasks]]
name = "base"
enabled = true
points = 30
gen = "gen_base"

[subtasks.generation]
enabled = true
test_count = 2
seed = "base-seed"
args = "--base"
expected_output_from_slow = true

[[subtasks]]
name = "full"
enabled = false
points = 70
depends_on = ["base"]
slow = "slow_full"
checker = "double_6"
time_limit_ms = 5000

[[tests]]
enabled = true
input = "1\n"
output = "1\n"
has_expected_output = true

[[tests]]
subtask = "full"
enabled = true
input = "2\n"
output = "2\n"
has_expected_output = true
)");

  const auto parsed = parse_problem_config_file(config.path);

  ASSERT_EQ(parsed["subtasks"].size(), 2);
  const auto &base = parsed["subtasks"][0];
  EXPECT_EQ(base["index"], 0);
  EXPECT_EQ(base["name"], "base");
  EXPECT_EQ(base["points"], 30);
  EXPECT_EQ(base["gen"], "gen_base");
  EXPECT_EQ(base["slow"], "slow");
  EXPECT_EQ(base["checker"], "token_checker");
  EXPECT_EQ(base["timeLimit"], 3000);
  EXPECT_TRUE(base["useGeneration"]);
  EXPECT_EQ(base["numTest"], 2);
  EXPECT_EQ(base["generatorSeed"], "base-seed");

  const auto &full = parsed["subtasks"][1];
  EXPECT_EQ(full["index"], 1);
  EXPECT_EQ(full["dependsOn"], nlohmann::json::array({"base"}));
  EXPECT_EQ(full["slow"], "slow_full");
  EXPECT_EQ(full["checker"], "double_6");
  EXPECT_EQ(full["timeLimit"], 5000);
  EXPECT_FALSE(full["useGeneration"]);

  EXPECT_EQ(parsed["tests"][0]["subtaskIndex"], 0);
  EXPECT_EQ(parsed["tests"][0]["subtask"], "base");
  EXPECT_EQ(parsed["tests"][1]["subtaskIndex"], 1);
  EXPECT_EQ(parsed["tests"][1]["subtask"], "full");
}

TEST(TestParseConfig, ExplicitSubtasksSurviveTomlWrite) {
  TemporaryConfig config(R"(
[problem]
name = "Round trip"
group = "Tests"

[run]
checker = "token_checker"
time_limit_ms = 3000

[language_config]
default = "py"

[[subtasks]]
name = "base"
points = 25
gen = "gen_base"

[subtasks.generation]
enabled = true
test_count = 3
seed = "seed"
expected_output_from_slow = true

[[subtasks]]
name = "full"
points = 75
depends_on = ["base"]
checker = "double_6"

[[tests]]
subtask = "full"
enabled = true
input = "1\n"
output = "1\n"
has_expected_output = true
)");
  const auto output = config.output_path();

  write_problem_config(output, parse_problem_config_file(config.path));
  const auto rewritten = parse_problem_config_file(output);

  EXPECT_TRUE(rewritten["explicitSubtasks"]);
  ASSERT_EQ(rewritten["subtasks"].size(), 2);
  EXPECT_EQ(rewritten["subtasks"][0]["gen"], "gen_base");
  EXPECT_EQ(rewritten["subtasks"][0]["numTest"], 3);
  EXPECT_EQ(rewritten["subtasks"][1]["dependsOn"], nlohmann::json::array({"base"}));
  EXPECT_EQ(rewritten["subtasks"][1]["checker"], "double_6");
  EXPECT_EQ(rewritten["tests"][0]["subtask"], "full");
  EXPECT_EQ(rewritten["languageConfig"]["default"], "py");
}

TEST(TestParseConfig, ImplicitSubtaskIsNotWrittenToLegacyConfig) {
  TemporaryConfig config(R"(
[problem]
name = "Legacy round trip"
group = "Tests"

[run]
checker = "token_checker"
time_limit_ms = 3000

[[tests]]
enabled = true
input = "1\n"
output = "1\n"
has_expected_output = true
)");
  const auto output = config.output_path();

  write_problem_config(output, parse_problem_config_file(config.path));
  const auto rewritten = parse_problem_config_file(output);

  EXPECT_FALSE(rewritten["explicitSubtasks"]);
  ASSERT_EQ(rewritten["subtasks"].size(), 1);
  EXPECT_EQ(rewritten["tests"][0]["subtask"], "");
}

TEST(TestParseConfig, CoreWriterPreservesUnrecognizedConfigData) {
  TemporaryConfig config(R"(
custom_top_level = "keep"

[problem]
name = "Unknown keys"
custom_problem_key = 17

[run]
checker = "token_checker"
time_limit_ms = 3000
custom_run_key = "keep"

[[tests]]
enabled = true
input = "1\n"
output = "1\n"
has_expected_output = true
custom_test_key = 42
)");
  const auto output = config.output_path();

  write_problem_config(output, parse_problem_config_file(config.path));
  const auto rewritten = parse_problem_config_file(output);

  EXPECT_EQ(rewritten["custom_top_level"], "keep");
  EXPECT_EQ(rewritten["problem"]["custom_problem_key"], 17);
  EXPECT_EQ(rewritten["run"]["custom_run_key"], "keep");
  EXPECT_EQ(rewritten["tests"][0]["custom_test_key"], 42);
}

TEST(TestParseConfig, ParsesConsumerConfigsWhenTaskRootIsProvided) {
  const char *task_root = std::getenv("CPCLI_TEST_TASK_ROOT");
  if (task_root == nullptr) {
    GTEST_SKIP() << "Set CPCLI_TEST_TASK_ROOT to run consumer config regression coverage";
  }

  size_t config_count = 0;
  for (const auto &entry : std::filesystem::directory_iterator(task_root)) {
    if (!entry.is_directory()) {
      continue;
    }
    const auto config_path = resolve_problem_config_path(entry.path());
    if (!std::filesystem::exists(config_path)) {
      continue;
    }
    SCOPED_TRACE(config_path.string());
    const auto parsed = parse_problem_config_file(config_path);
    EXPECT_TRUE(parsed.contains("subtasks"));
    EXPECT_FALSE(parsed["subtasks"].empty());
    ++config_count;
  }
  EXPECT_GT(config_count, 0);
}
