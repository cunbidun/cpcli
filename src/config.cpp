
#include "config.hpp"
#include "operations.hpp"
#include "util/util.hpp"

using std::cout;

json read_problem_config(const string &path, std::filesystem::path template_path) {
  if (!check_file(path, "")) {
    cout << "Error, no config.json file found. Please create one\n";
    cout << "Sample config:\n";
    std::vector<string> command;
    command.push_back("cat");
    command.push_back("\"" + template_path.string() + "\"");
    system(join(command).c_str());
    clean_up();
    exit(120384);
  }
  std::ifstream input(path);
  json j;
  input >> j;
  return j;
}

bool validate_problem_config(const json &obj) { return true; }

json read_project_config(const string &path) {
  std::ifstream input(path);
  json j;
  input >> j;
  return j;
}

bool validate_project_config(const json &obj) { return true; }
