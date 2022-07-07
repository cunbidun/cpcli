#ifndef _cpcli_operations_hpp_
#define _cpcli_operations_hpp_

#include <filesystem>
#include <iostream>
#include <string>
#include <unistd.h>
#include <vector>

#include "nlohmann/json.hpp"

using json = nlohmann::json;

using std::string;
namespace fs = std::filesystem;

// TODO add docs
void print_usage();

// TODO add docs
bool check_file(std::filesystem::path path, const string &error_message);

// TODO add docs
json read_problem_config(fs::path path, fs::path temp_config_path);

// TODO add docs
json read_project_config(fs::path path);

// TODO add docs
int compile_cpp(fs::path &cache_dir,
                bool use_cache,
                const string &c_complier,
                fs::path &path,
                const string &compiler_flags,
                const string &binary_name);

int create_new_task(json project_conf);

void print_duration(std::chrono::high_resolution_clock::time_point t_start);

// TODO add docs
void print_report(const string report_name, bool passed, bool rte, bool tle, bool wa, long long runtime);

// TODO add docs
void edit_config(fs::path root_dir, fs::path &template_dir, string &frontend_exec);

bool compile_headers(fs::path precompiled_dir, const string &cc, const string &flag, const string &debug);

#endif
