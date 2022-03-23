#ifndef _cpcli_operations_hpp_
#define _cpcli_operations_hpp_

#include <filesystem>
#include <iostream>
#include <string>
#include <unistd.h>
#include <vector>

using std::string;
namespace fs = std::filesystem;

// TODO add docs
void print_usage();

// TODO add docs
int compile_cpp(std::filesystem::path &cache_dir,
                bool use_cache,
                const string &c_complier,
                fs::path &path,
                const string &compiler_flags,
                const string &binary_name);

// TODO add docs
int clean_up(int first_time = 0);

// TODO add docs
void print_report(const string report_name, bool passed, bool rte, bool tle, bool wa, long long runtime);

// Methods handle the SIGNINT signal
void sigint();

// TODO add docs
void edit_config(fs::path root_dir, fs::path &template_dir, fs::path &frontend_path);

#endif
