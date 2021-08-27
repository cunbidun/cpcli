#include <fstream>

#include "cpcli_operations.hpp"
#include "cpcli_problem_config.hpp"
#include "cpcli_utils.hpp"

using std::cout;

nlohmann::json read_problem_config(const string &path, std::filesystem::path template_path) {
  if (!check_file(path, "")) {
    cout << "Error, no config.json file found. Please create one\n";
    cout << "Sample config:\n";
    std::vector<string> command;
    command.push_back("cat");
    command.push_back("\"" + template_path.string() + "\"");
    system(join(command).c_str());
    clean_up();
    exit(1);
  }
  std::ifstream input(path);
  nlohmann::json j;
  input >> j;
  return j;
}

bool validate_problem_config(const nlohmann::json &obj) {
  // TODO implement this method
  return true;
}
