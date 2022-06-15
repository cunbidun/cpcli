#include <gtest/gtest.h>
#include <util/util.hpp>

#include <filesystem>
#include <string>
#include <vector>

namespace fs = std::filesystem;
using std::string;

TEST(TestUtil, TestEmptyFile) {
  auto cache_dir = fs::temp_directory_path() / "cpcli_test";
  string file_name = gen_string_length_20();
  EXPECT_EQ(file_name.size(), 20) << "gen_string_length_20 doesn't return string with length of 20";
  auto file_path = cache_dir / file_name;
  create_empty_file(file_path);
  EXPECT_EQ(is_empty_file(file_path), true);
}

TEST(TestUtil, TestJoin) {
  std::vector<string> x = {"a", "b", "c"};
  EXPECT_EQ(join(x), "a b c") << "join on ['a' 'b' 'c'] not working.";
}
