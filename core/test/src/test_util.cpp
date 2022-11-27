#include "utils.hpp"
#include <gtest/gtest.h>

#include <filesystem>
#include <string>
#include <vector>

namespace fs = std::filesystem;
using std::string;

TEST(test_utils, test_gen_string_length_20) {
  string s = gen_string_length_20();
  EXPECT_EQ(s.size(), 20);
}

TEST(TestUtil, TestEmptyFile) {
  auto cache_dir = std::filesystem::temp_directory_path() / "cpcli_test";
  string file_name = gen_string_length_20();
  auto file_path = cache_dir / file_name;
  create_empty_file(file_path);
  EXPECT_EQ(is_empty_file(file_path), true);
}

TEST(TestUtil, TestJoin) {
  std::vector<string> x = {"a", "b", "c"};
  EXPECT_EQ(join(x), "a b c") << "join on ['a' 'b' 'c'] not working.";
}

TEST(TestUtil, TestJoinEmpty) {
  std::vector<string> x;
  EXPECT_EQ(join(x), "") << "join on empty vector not working.";
}

TEST(TestUtil, TestJoinOneElement) {
  std::vector<string> x = {"a"};
  EXPECT_EQ(join(x), "a") << "join on one element vector not working.";
}

TEST(TestUtil, TestSystemWarper) {
  int output = system_warper("ls");
  EXPECT_EQ(output, 0);
}

TEST(TestUtil, TestSystemWarperInvalidCommand) {
  int output = system_warper("invalid_command");
  EXPECT_NE(output, 0);
}

TEST(TestUtil, TestTrim) {
  string s = "  hello  ";
  trim(s);
  EXPECT_EQ(s, "hello");
}

TEST(TestUtil, TestTrimEmpty) {
  string s = "";
  trim(s);
  EXPECT_EQ(s, "");
}