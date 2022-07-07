#ifndef _cpcli_util_hpp_
#define _cpcli_util_hpp_

#include "color.hpp"
#include <algorithm>
#include <cctype>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <locale>
#include <random>
#include <signal.h>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <time.h>
#include <vector>

using std::string;

// TODO add docs
bool is_empty_file(const string &path);

// TODO add docs
string join(std::vector<string> &v);

// TODO add docs
string gen_string_length_20();

// trim from start (in place)
static inline void ltrim(std::string &s) {
  s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) { return !std::isspace(ch); }));
}

// trim from end (in place)
static inline void rtrim(std::string &s) {
  s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(), s.end());
}

// trim from both ends (in place)
static inline void trim(std::string &s) {
  ltrim(s);
  rtrim(s);
}

// TODO add docs
int clean_up();

// Methods handle the SIGNINT signal
void hande_sigint();

// trim from start (copying)
static inline std::string ltrim_copy(std::string s) {
  ltrim(s);
  return s;
}

// trim from end (copying)
static inline std::string rtrim_copy(std::string s) {
  rtrim(s);
  return s;
}

// trim from both ends (copying)
static inline std::string trim_copy(std::string s) {
  trim(s);
  return s;
}
// TODO add docs
bool create_empty_file(const string &path);

// TODO add docs
void print_file(string path, bool truncate);

// TODO add docs
bool compare_files(const std::filesystem::path &filename1, const std::filesystem::path &filename2);

// TODO add docs
int system_warper(const string &command);

#endif
