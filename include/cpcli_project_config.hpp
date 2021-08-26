#ifndef _cpcli_project_config_h_
#define _cpcli_project_config_h_

#include <nlohmann/json.hpp>
#include <string>
#include <vector>

using std::string;

// TODO add docs
nlohmann::json read_project_config(const string &path);

// TODO validate project config
bool validate_project_config(const nlohmann::json &obj);

#endif
