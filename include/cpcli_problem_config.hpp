#ifndef _cpcli_problem_config_h_
#define _cpcli_problem_config_h_

#include <filesystem>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

using std::string;

// TODO add docs
nlohmann::json read_problem_config(const string &path, std::filesystem::path temp_config_path);

// TODO validate project config
bool validate_problem_config(const nlohmann::json &obj);

#endif
