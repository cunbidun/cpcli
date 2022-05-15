#include <fstream>
#include <iostream>
#include <operations.hpp>

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
  fs::path path = precompiled_dir / "tmp.cpp";
  fs::path compile_header = precompiled_dir / "cpp_compile_flag";
  fs::path compile_debug = precompiled_dir / "cpp_debug_flag";
  fs::remove_all(compile_header);
  fs::remove_all(compile_debug);
  fs::create_directory(compile_header);
  fs::create_directory(compile_debug);
  std::ofstream file(path);

  file << "#include <bits/stdc++.h>" << endl;
  file << "using namespace std;" << endl;
  file << "int main() {}" << endl;
  file.close();
  string command = cc + " file.cpp -H 2>&1 | grep bits/stdc++.h";
  exec(command.c_str());
}