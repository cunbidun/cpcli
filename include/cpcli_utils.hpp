#ifndef _cpcli_utils_h_
#define _cpcli_utils_h_

#include <algorithm>
#include <cctype>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <locale>
#include <random>
#include <sstream>
#include <string>
#include <time.h>
#include <vector>

using std::string;

static inline string DASH_SEPERATOR = "\033[1;34m---------------------------\033[0m"; // blue color
static inline string EQUA_SEPERATOR = "===========================";

// TODO add docs
bool check_dir(const string &name, const string &error_message);

// TODO add docs
bool check_file(const string &name, const string &error_message);

// TODO add docs
string join(std::vector<string> &v);

// TODO add docs
string gen_string_length_20();

// trim from end (in place)
static inline void rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}
// TODO add docs
bool create_empty_file(const string &path);

// trim from end (copying)
static inline string rtrim_copy(string s) {
  rtrim(s);
  return s;
}

// TODO add docs
void print_file(string path, bool truncate);

// TODO add docs
bool compare_files(const std::filesystem::path &filename1, const std::filesystem::path &filename2);

// TODO add docs
int system_wraper(const string &command);
#endif
