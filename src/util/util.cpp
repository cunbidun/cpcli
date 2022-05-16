
#include "util/util.hpp"
#include "constant.hpp"
#include "operations.hpp"
#include "spdlog/spdlog.h"

using std::cout, std::string;

bool check_file(std::filesystem::path path, const string &error_message) {
  if (std::filesystem::exists(path)) {
    return true;
  } else {
    if (error_message != "") {
      spdlog::error(error_message);
      clean_up();
      exit(FILE_NOT_FOUND_ERR);
    }
    return false;
  }
}

bool create_empty_file(const string &path) {
  std::ofstream tmp(path);
  tmp.close();
  return true;
}

string join(std::vector<string> &v) {
  std::stringstream ss;
  for (int i = 0; i < (int)v.size(); ++i) {
    if (i != 0) {
      ss << ' ';
    }
    ss << v[i];
  }
  return ss.str();
}

void print_file(string path, bool truncate) {
  std::stringstream buffer;
  buffer << std::ifstream(path).rdbuf();
  if (truncate && buffer.str().size() >= 100) {
    string buff = rtrim_copy(buffer.str());
    cout << buff.substr(0, 35) << "..." << buff.substr(buff.size() - 35, 35) << '\n';
  } else {
    cout << rtrim_copy(buffer.str()) << '\n';
  }
}

string gen_string_length_20() {
  std::mt19937_64 rng(std::chrono::steady_clock::now().time_since_epoch().count());
  string s = "";
  for (int i = 0; i < 20; i++) {
    s += static_cast<char>('a' + rng() % 26);
  }
  return s;
}

template <typename InputIterator1, typename InputIterator2>
bool range_equal(InputIterator1 first1, InputIterator1 last1, InputIterator2 first2, InputIterator2 last2) {
  while (first1 != last1 && first2 != last2) {
    if (*first1 != *first2) {
      return false;
    }
    ++first1;
    ++first2;
  }
  return (first1 == last1) && (first2 == last2);
}

bool compare_files(const std::filesystem::path &filename1, const std::filesystem::path &filename2) {
  std::filesystem::current_path(filename1.parent_path());
  std::ifstream file1(filename1.filename());

  std::filesystem::current_path(filename2.parent_path());
  std::ifstream file2(filename2.filename());

  std::istreambuf_iterator<char> begin1(file1);
  std::istreambuf_iterator<char> begin2(file2);

  std::istreambuf_iterator<char> end;

  return range_equal(begin1, end, begin2, end);
}

int system_warper(const string &command) {
  int status = std::system(command.c_str());
  if (WIFSIGNALED(status) && (WTERMSIG(status) == SIGINT)) {
    sigint();
  }
  return WEXITSTATUS(status);
}