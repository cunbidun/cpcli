#ifndef _cpcli_config_hpp_
#define _cpcli_config_hpp_

#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

using std::string;
using json = nlohmann::json;

// TODO add docs
json read_problem_config(const string &path, std::filesystem::path temp_config_path);

// TODO validate project config
bool validate_problem_config(const json &obj);

// TODO add docs
json read_project_config(const string &path);

// TODO validate project config
bool validate_project_config(const json &obj);
#endif
