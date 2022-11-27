#include "utils.hpp"
#include "constant.hpp"
#include "spdlog/spdlog.h"

using std::cout, std::string;
namespace fs = std::filesystem;

bool is_empty_file(const string &path) {
  std::ifstream file(path);
  return file.peek() == std::ifstream::traits_type::eof();
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

string read_file_to_str(const std::filesystem::path &path) {
  std::stringstream buffer;
  buffer << std::ifstream(path).rdbuf();
  return buffer.str();
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

void handle_sigint() {
  cout << std::endl;
  cout << termcolor::red << termcolor::bold << "SIGINT encountered\n";
  clean_up();
  exit(0);
}

int clean_up() {
  // Remove binary and ___test_case directory every time
  std::filesystem::remove("solution");
  std::filesystem::remove("checker");
  std::filesystem::remove("gen");
  std::filesystem::remove("slow");
  std::filesystem::remove("interactor");
  std::filesystem::remove_all("___test_case");
  return 0;
}

int system_warper(const string &command) {
  int status = std::system(command.c_str());
  if (WIFSIGNALED(status) && (WTERMSIG(status) == SIGINT)) {
    handle_sigint();
  }
  return WEXITSTATUS(status);
}
