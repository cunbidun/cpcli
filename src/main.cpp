#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <set>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

namespace fs = std::filesystem;

#include "color.hpp"
#include "cpcli_operations.hpp"
#include "cpcli_problem_config.hpp"
#include "cpcli_project_config.hpp"
#include "cpcli_utils.hpp"

using std::cout;
using std::endl;
using std::string;
using std::to_string;
using json = nlohmann::json;

fs::path root_dir;   // where source files and problem_config file located
fs::path output_dir; // where source will be put for submission
fs::path binary_dir; // where source will be put for submission
fs::path project_config_path, problem_config_path, solution_file_path;

json project_config, config;

string compiler_flags = "cpp_compile_flag";
string generator_seed;

// TODO add actual usage
void print_usage() {
  cout << "the number of parmameter is not correct!" << endl;
  cout << "usage: cpcli <path/to/folder> <path/to/project_config.json> [op_num]" << endl;
  cout << "op_num = 0 (default):  run normally" << endl;
  cout << "op_num = 1:            run with debug flags " << endl;
  exit(0);
}

std::chrono::system_clock::time_point t_start;

// TODO add return code
int main(int argc, char *argv[]) {

  if (argc != 3 && argc != 4) {
    print_usage();
  }

  t_start = std::chrono::high_resolution_clock::now();
  root_dir = std::filesystem::absolute(argv[1]);            // set the root directory to argv[1]
  project_config_path = std::filesystem::absolute(argv[2]); // set the root directory to argv[2]

  check_dir(root_dir, "root directory not found!"); // check if the root_dir exists
  std::filesystem::current_path(root_dir);          // change directory to root_dir
  clean_up(1);                                  // clean up the root directory for the first time

  project_config = read_project_config(project_config_path); // reade the project config into a json object
  validate_project_config(project_config);

  // TODO pass as K-V args
  if (argc == 4) {
    compiler_flags = "cpp_debug_flag";
    project_config["use_cache"] = 0; // don't use cache for debuging
  }

  output_dir = std::filesystem::absolute(project_config["output_dir"]);
  check_dir(output_dir, "output directory not found!"); // check if the output exists

  binary_dir = std::filesystem::absolute(project_config["binary_dir"]);
  check_dir(binary_dir, "binary_dir not found!");

  if (project_config["use_precompiled_header"]) {
    // TODO validate precompiled_header

    // check if precompiled is specified in project settings
    if (project_config["precompiled_header_dir"] == nullptr) {
      cout << "no precompiled header directory specified!";
      exit(1);
    }

    fs::path precompiled_dir = std::filesystem::absolute(project_config["precompiled_header_dir"]);

    // TODO revamp precompiled_dir structure
    fs::path precompiled_path = precompiled_dir / "stdc++.h";
    check_file(precompiled_dir / "stdc++.h.gch", "precompiled header not found!");

    fs::path precompiled_debug_path = precompiled_dir / "debug" / "stdc++.h";
    check_file(precompiled_dir / "debug" / "stdc++.h.gch", "precompiled debug header not found!");

    project_config["cpp_compile_flag"] = project_config["cpp_compile_flag"].get<string>() + " " + "-include" + " " + precompiled_path.string();
    project_config["cpp_debug_flag"] = project_config["cpp_debug_flag"].get<string>() + " " + "-include" + " " + precompiled_debug_path.string();
  }

  check_file(project_config_path, "project config file not found"); // check if the project_config.json exists

  problem_config_path = root_dir / "config.json";
  check_file(problem_config_path, "problem config file not found");
  config = read_problem_config(problem_config_path); // reade the project config into a json object
  validate_problem_config(config);

  // ----------------------------- COMPILE START ----------------------------

  fs::path cache_dir = "";

  bool use_cache = project_config["use_cache"];
  if (use_cache) {
    cache_dir = std::filesystem::absolute("/tmp/cpcli") / to_string(std::hash<std::string>()(root_dir));
    if (mkdir(cache_dir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == -1) {
      if (errno == EEXIST) {
      } else {
        // something else
        cout << termcolor::red << "cannot create cache folder: " << strerror(errno) << std::endl;
        throw std::runtime_error(strerror(errno));
      }
    }
  }

  // use slow solution for generate correct output
  // requre slow.cpp
  if (config["knowGenAns"]) {
    fs::path slow_file_path = root_dir / "slow.cpp";
    check_file(slow_file_path, "brute force solution file not found!");
    compile_cpp(cache_dir, use_cache, slow_file_path, project_config[compiler_flags], "slow");
  }

  if (config["interactive"]) {
    fs::path interactor_file_path = root_dir / "slow.cpp";
    check_file(interactor_file_path, "interactor file not found!");
    compile_cpp(cache_dir, use_cache, interactor_file_path, project_config[compiler_flags], "interactor");
  } else {
    if (config["checker"] != "custom") {
      fs::path checker_bin_path = binary_dir / "checker" / config["checker"];
      check_file(checker_bin_path, "checker binary not found!");
      copy_file(checker_bin_path, root_dir / "checker");
    } else {
      fs::path checker_file_path = root_dir / "slow.cpp";
      check_file(checker_file_path, "checker file not found!");
      compile_cpp(cache_dir, use_cache, checker_file_path, project_config["cpp_compile_flag"], "checker");
    }
    cout << termcolor::cyan << termcolor::bold << "Using " << config["checker"].get<string>() << " checker!" << termcolor::reset << '\n';
  }

  if (config["useGeneration"]) {
    fs::path gen_file_path = root_dir / "gen.cpp";
    check_file(gen_file_path, "gen file not found!");
    compile_cpp(cache_dir, use_cache, gen_file_path, project_config["cpp_compile_flag"], "gen");
    generator_seed = config["generatorSeed"];
    if (config["generatorSeed"].get<string>().size() == 0) {
      generator_seed = gen_string_length_20();
    }
    cout << termcolor::yellow << termcolor::bold << "Stress testing with seed \'" << generator_seed << "\'" << termcolor::reset << '\n';
  }

  solution_file_path = root_dir / "solution.cpp"; // NOTE support c++ for now
  check_file(solution_file_path, "solution file not found");
  compile_cpp(cache_dir, use_cache, solution_file_path, project_config[compiler_flags], "solution");
  copy_file(solution_file_path, output_dir / "solution.cpp", fs::copy_options::overwrite_existing); // copy solution file to output dir for submission

  cout << DASH_SEPERATOR << '\n';
  // ------------------------------ COMPILE END -----------------------------

  // ------------------------ GENERATING TESTS START ------------------------

  {
    std::filesystem::current_path(root_dir);
    mkdir("___test_case", 0777);
    std::filesystem::current_path("___test_case");
    for (json test : config["tests"]) {
      if (test["active"]) {
        if (test["input"] != nullptr) {
          std::ofstream inf(to_string(test["index"]) + ".in");
          inf << test["input"].get<string>();
        }
        if (test["output"] != nullptr) {
          std::ofstream ouf(to_string(test["index"]) + ".out");
          ouf << test["output"].get<string>();
          ouf.close();
        }
      }
    }

    if (config["useGeneration"]) {
      string command = (root_dir / "gen").string() + " " + generator_seed + " " + to_string(config["numTest"].get<int>());
      int status = std::system(command.c_str());
      if (status != 0) {
        cout << termcolor::red << termcolor::bold << "generator run time error" << termcolor::reset << endl;
        clean_up();
        exit(1);
      }
    }
  }
  // ------------------------ GENERATING TESTS END --------------------------

  bool all_passed = 1, all_rte = 0, all_tle = 0, all_wa = 0;
  long long all_runtime = 0;

  // ----------------------------- TESTS START ------------------------------
  {
    std::filesystem::current_path(root_dir);
    long long time_limit = config["timeLimit"].get<long long>();
    auto tests_folder_dir = std::filesystem::path("___test_case"); // set the root directory to argv[1]
    create_empty_file(tests_folder_dir / "___na___");
    //--- filenames are unique so we can use a set
    std::set<fs::path> sorted_by_name;

    for (auto &entry : fs::directory_iterator(tests_folder_dir)) {
      if (entry.path().extension() == ".in") {
        sorted_by_name.insert(entry.path());
      }
    }

    for (const auto &entry : sorted_by_name) {
      bool passed = 1, undecided = 0, rte = 0, tle = 0, wa = 0;
      long long runtime = 0;
      const auto test_id = entry.stem().string();
      const auto actual_file = tests_folder_dir / (test_id + ".actual");
      const auto out_file = tests_folder_dir / (test_id + ".out");
      const auto res_file = tests_folder_dir / (test_id + ".res");

      create_empty_file(res_file);

      // --------- test id ------------
      if (test_id[0] != 'S') {
        cout << termcolor::cyan << termcolor::bold << "Test #" << test_id << ": " << termcolor::reset;
      } else {
        cout << termcolor::yellow << termcolor::bold << "Test #" << test_id << ": " << termcolor::reset;
      }

      {
        string command = "./solution < " + entry.string() + " > " + actual_file.string();

        auto t0 = std::chrono::high_resolution_clock::now();
        int status = system(command.c_str());
        auto t1 = std::chrono::high_resolution_clock::now();

        runtime = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
        tle = (runtime > time_limit);

        if (status != 0) {
          rte = 1;
          passed = 0;
        }
      }
      if (config["knowGenAns"]) {
        string command = "./slow < " + entry.string() + " > " + out_file.string();
        int status = system(command.c_str());
        if (status != 0) {
          cout << termcolor::red << termcolor::bold << "slow solution run time error" << termcolor::reset << endl;
          clean_up();
          exit(1);
        }
      }

      {
        string out_file_str = out_file.string();
        if (!check_file(out_file, "")) {
          out_file_str = tests_folder_dir / "___na___";
        }

        string command = "./checker " + entry.string() + "  " + out_file_str + " " + actual_file.string() + " " + res_file.string() + " > /dev/null 2>&1";
        int status = WEXITSTATUS(system(command.c_str()));
        if (status != 0) {
          passed = 0;
        }
        if (status == 3) {
          undecided = 1;
        }
        wa = !passed && !undecided;
      }

      all_passed &= passed;
      all_rte |= rte;
      all_tle |= tle;
      all_wa |= wa;
      all_runtime = std::max(all_runtime, runtime);
      if (passed && config["hideAcceptedTest"]) {
        cout << termcolor::green << termcolor::bold << "accepted" << termcolor::reset << '\n';
      } else {
        bool truncate = config["truncateLongTest"].get<bool>();
        cout << '\n';
        // --------- input --------------
        cout << "Input:" << '\n';
        print_file(entry.string(), truncate);
        cout.flush();

        // --------- expected output --------------
        if (check_file(out_file, "")) {
          cout << "Expected output:" << '\n';
          print_file(out_file, truncate);
        }

        // --------- exe output --------------
        {
          cout << "Execution output:" << '\n';
          cout.flush();
          print_file(actual_file, truncate);
        }

        // --------- Verdict --------------
        print_report("Verdict", passed, rte, tle, wa, runtime);
      }
      cout << DASH_SEPERATOR << '\n';
      std::filesystem::current_path(root_dir);

      if (config["stopAtWrongAnswer"] && (wa || rte || tle)) {
        print_report("Fail detected", all_passed, all_rte, all_tle, all_wa, all_runtime);
        clean_up();
        return 0;
      }
    }
  }
  // ------------------------------ TESTS END -------------------------------

  // ------------------------------ PRINT REPORT START -------------------------------
  {
    cout << EQUA_SEPERATOR << '\n';
    print_report("Results", all_passed, all_rte, all_tle, all_wa, all_runtime);
    clean_up();
  }

  // ------------------------------ PRINT REPORT END -------------------------------
}
