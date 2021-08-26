#include <fstream>
#include <iostream>

#include "cpcli_project_config.hpp"

nlohmann::json read_project_config(const string &path) {
  std::ifstream input(path);
  nlohmann::json j;
  input >> j;
  return j;
}

bool validate_project_config(const nlohmann::json &obj) {
  // TODO implement this method
  return true;
}
