#ifndef _cpcli_compile_h_
#define _cpcli_compile_h_

#include <filesystem>
#include <string>
using std::string;

// TODO add docs
int compile_cpp(std::filesystem::path &cache_dir,
                bool use_cache,
                const string &c_complier,
                std::filesystem::path &path,
                const string &compiler_flags,
                const string &binary_name);

// TODO add docs
int clean_up(int first_time = 0);

// TODO add docs
void print_report(const string report_name,
                  bool passed,
                  bool rte,
                  bool tle,
                  bool wa,
                  long long runtime);

// Thie methods handle the SIGNINT signal
void sigint();

// TODO add docs
void edit_config(std::filesystem::path root_dir,
                 std::filesystem::path &template_dir,
                 std::filesystem::path &frontend_path);

#endif
