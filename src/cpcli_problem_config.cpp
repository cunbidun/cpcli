#include <fstream>

#include "cpcli_problem_config.hpp"

nlohmann::json read_problem_config(const string &path) {
  std::ifstream input(path);
  nlohmann::json j;
  input >> j;
  return j;
}

bool validate_problem_config(const nlohmann::json &obj) {
  // TODO implement this method
  return true;
}
