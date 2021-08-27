#include "color.hpp"
#include "cpcli_operations.hpp"
#include "cpcli_utils.hpp"

#include <filesystem>
#include <unistd.h>
#include <vector>

using std::cout;
using std::endl;
using std::string;

extern std::chrono::high_resolution_clock::time_point t_start;

int clean_up(int first_time) {
  std::filesystem::remove("solution");
  std::filesystem::remove("checker");
  std::filesystem::remove("gen");
  std::filesystem::remove("slow");
  std::filesystem::remove("interactor");
  std::filesystem::remove("printer");
  std::filesystem::remove_all("___test_case");

  if (!first_time) {
    auto t_end = std::chrono::high_resolution_clock::now();
    long long total_time = std::chrono::duration_cast<std::chrono::milliseconds>(t_end - t_start).count();
    cout << termcolor::magenta << termcolor::bold << "All testing finished in " << total_time << " ms" << endl;
  }
  return 0;
}

int compile_cpp(std::filesystem::path &cache_dir, bool use_cache, std::filesystem::path &path, const string &compiler_flags, const string &binary_name) {
  std::filesystem::path binary_cache_dir = cache_dir / binary_name;
  std::filesystem::path file_cache_dir = cache_dir / path.filename();

  if (use_cache) {
    if (check_file(binary_cache_dir, "") && compare_files(path, file_cache_dir)) {
      copy_file(binary_cache_dir, path.parent_path() / binary_name, std::filesystem::copy_options::overwrite_existing); // copy solution file to output dir for submission
      return 0;
    }
  }

  std::filesystem::current_path(path.parent_path());
  std::vector<string> command;

  // build command
  command.push_back("g++");
  command.push_back(compiler_flags);
  command.push_back("-o");
  command.push_back(binary_name);
  command.push_back("\"" + path.string() + "\"");

  int status = system_wraper(join(command));

  if (status != 0) {
    clean_up();
    exit(1); // TODO set correct return code
  }

  if (use_cache) {
    copy_file(path.parent_path() / binary_name, binary_cache_dir, std::filesystem::copy_options::overwrite_existing); // copy solution file to output dir for submission
    copy_file(path, file_cache_dir, std::filesystem::copy_options::overwrite_existing);                               // copy solution file to output dir for submission
  }
  return status;
}

void print_report(const string report_name, bool passed, bool rte, bool tle, bool wa, long long runtime) {
  using std::cout;
  cout << report_name << ": ";
  if (rte != 0) {
    cout << termcolor::red << termcolor::bold << "runtime error" << termcolor::reset << '\n';
  } else {
    auto run_time_color = termcolor::green;
    if (tle) {
      run_time_color = termcolor::red;
      cout << termcolor::red << termcolor::bold << "time limit exceeded" << termcolor::reset;
    } else if (passed) {
      cout << termcolor::green << termcolor::bold << "accepted" << termcolor::reset;
    } else if (wa) {
      cout << termcolor::red << termcolor::bold << "wrong answer" << termcolor::reset;
    } else {
      cout << termcolor::yellow << termcolor::bold << "undecided" << termcolor::reset;
    }
    cout << " in " << run_time_color << termcolor::bold << runtime << termcolor::reset << " ms\n";
  }
}

void sigint() {
  cout << endl;
  cout << termcolor::red << termcolor::bold << "SIGINT encoutered\n";
  clean_up();
  exit(0);
}
