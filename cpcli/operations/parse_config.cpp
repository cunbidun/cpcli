#include "constant.hpp"
#include "operations.hpp"
#include "utils.hpp"
#include <istream>

using std::cout;

bool validate_problem_config(const json &obj) { return true; }

json read_problem_config(fs::path path, fs::path template_path) {
  if (path.empty() || !check_file(path, "")) {
    cout << "Error, no config.json file found. Please create one\n";
    cout << "Sample config:\n";
    print_file(template_path.string(), false);
    clean_up();
    exit(FILE_NOT_FOUND_ERR);
  }
  std::ifstream input(path);
  json j;
  input >> j;
  if (!validate_problem_config(j)) {
    cout << "Invalid problem configuration\n";
    cout << "Sample config:\n";
    print_file(template_path.string(), false);
    clean_up();
    exit(INVALID_CONFIG);
  }
  return j;
}

bool validate_project_config(const json &obj) { return true; }

json read_project_config(fs::path path) {
  if (path.empty() || !check_file(path, "")) {
    cout << "Error, no project configuration file found. Please create one from template\n";
    clean_up();
    exit(FILE_NOT_FOUND_ERR);
  }
  std::ifstream input(path);
  json j;
  input >> j;
  if (!validate_project_config(j)) {
    cout << "Invalid project configuration\n";
    clean_up();
    exit(INVALID_CONFIG);
  }
  return j;
}
