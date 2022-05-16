#include "operations.hpp"
#include "spdlog/spdlog.h"
#include "util/util.hpp"

#include <fstream>
#include <iostream>

using std::endl;

string exec(const char *cmd) {
  std::array<char, 128> buffer;
  std::string result;
  std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
  if (!pipe) {
    throw std::runtime_error("popen() failed!");
  }
  while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
    result += buffer.data();
  }
  return result;
}

bool compile_headers(fs::path precompiled_dir, const string &cc, const string &flag, const string &debug) {
  fs::create_directory(precompiled_dir);
  fs::current_path(precompiled_dir);
  fs::path compile_header = precompiled_dir / "cpp_compile_flag";
  fs::path debug_header = precompiled_dir / "cpp_debug_flag";
  fs::remove_all(compile_header);
  fs::remove_all(debug_header);
  fs::create_directory(compile_header);
  fs::create_directory(debug_header);

  fs::path tmp_path = precompiled_dir / "tmp.cpp";
  fs::remove(tmp_path);
  std::ofstream file(tmp_path);
  file << "#include <bits/stdc++.h>" << endl;
  file << "using namespace std;" << endl;
  file << "int main() {}" << endl;
  file.close();

  string command = cc + " " + tmp_path.string() + " -H 2>&1 | grep bits/stdc++.h | awk 'END{ print $NF }'";
  string header_path = trim_copy(exec(command.c_str()));
  spdlog::debug("Header path is: " + header_path);

  fs::copy(header_path, precompiled_dir / "stdc++.h", fs::copy_options::overwrite_existing);

  // compile header with normal flags
  command = cc + " " + flag + " stdc++.h";
  std::system(command.c_str());
  fs::rename("stdc++.h.gch", compile_header / "stdc++.h.gch");

  // compile header with debug flags
  command = cc + " " + debug + " stdc++.h";
  std::system(command.c_str());
  fs::rename("stdc++.h.gch", debug_header / "stdc++.h.gch");

  fs::remove(tmp_path);
  fs::remove(precompiled_dir / "a.out");
  fs::remove(precompiled_dir / "stdc++.h");
}