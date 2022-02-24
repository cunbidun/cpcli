#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <set>
#include <signal.h>
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
#include "nlohmann/json.hpp"

using std::cout;
using std::endl;
using std::string;
using std::to_string;
using json = nlohmann::json;

fs::path root_dir;   // where source files and problem_config file located
fs::path output_dir; // where source will be put for submission
fs::path binary_dir; // where source will be put for submission
fs::path cpcli_dir;  // where source will be put for submission
fs::path project_config_path, problem_config_path, solution_file_path;

json project_config, config;

string testlib_compiler_flag;
string compiler_flags = "cpp_compile_flag";
string generator_seed;

// TODO add actual usage
void print_usage() {
  cout << "the number of parmameter is not correct!" << endl;
  cout << "usage: cpcli <path/to/folder> <path/to/project_config.json> [op_num]"
       << endl;
  cout << "op_num = 0 (default):  run normally" << endl;
  cout << "op_num = 1:            run with debug flags " << endl;
  cout << "op_num = 2:            run with terminal" << endl;
  cout << "op_num = 3:            test frontend" << endl;
  cout << "op_num = 4:            archive task" << endl;
  cout << endl;

  cout << "usage: cpcli <path/to/project_config.json> (for create new task)"
       << endl;
  exit(0);
}

std::chrono::high_resolution_clock::time_point t_start;

// TODO add return code
int main(int argc, char *argv[]) {
  // TODO implement SIGINT
  signal(SIGINT, [](int sig) { sigint(); });

  t_start = std::chrono::high_resolution_clock::now();

  if (argc == 2) { // create new task
    string new_task = "___n3w_t4sk";
    project_config_path =
        fs::absolute(argv[1]); // set the root directory to argv[2]
    project_config = read_project_config(
        project_config_path); // reade the project config into a json object
    validate_project_config(project_config);
    fs::path task_dir = fs::absolute(project_config["task_dir"].get<string>());
    fs::path frontend_path =
        fs::absolute(project_config["frontend_path"].get<string>());
    fs::path template_dir =
        fs::absolute(project_config["template_dir"].get<string>());

    fs::create_directory(task_dir / new_task);
    fs::current_path(task_dir / new_task);
    root_dir = fs::current_path();

    fs::copy_file(template_dir / "solution.template", root_dir / "solution.cpp",
                  fs::copy_options::overwrite_existing);
    fs::copy_file(template_dir / "config.template", root_dir / "config.json",
                  fs::copy_options::overwrite_existing);
    edit_config(task_dir / new_task, template_dir, frontend_path);
    config = read_problem_config(
        root_dir / "config.json",
        template_dir /
            "config.template"); // reade the project config into a json object
    validate_problem_config(config);
    string name = config["name"].get<string>();

    fs::current_path(root_dir.parent_path());

    if (name.size() == 0 || check_dir(name, "")) {
      fs::remove_all(new_task);
    } else {
      fs::rename(new_task, name);
    }
    return 0;
  }

  if (argc != 3 && argc != 4) {
    print_usage();
  }

  root_dir = fs::absolute(argv[1]); // set the root directory to argv[1]
  project_config_path =
      fs::absolute(argv[2]); // set the root directory to argv[2]

  check_dir(root_dir,
            "root directory not found!"); // check if the root_dir exists
  fs::current_path(root_dir);             // change directory to root_dir
  clean_up(1); // clean up the root directory for the first time

  project_config = read_project_config(
      project_config_path); // reade the project config into a json object
  validate_project_config(project_config);

  cpcli_dir = fs::absolute(std::getenv("CPCLI_PATH"));
  output_dir = fs::absolute(project_config["output_dir"]);

  testlib_compiler_flag = project_config["cpp_compile_flag"].get<string>();
  if (project_config["include_dir"] != nullptr &&
      project_config["include_dir"].get<string>().size() != 0) {
    string include_dir =
        "\"" + project_config["include_dir"].get<string>() + "\"";
    testlib_compiler_flag = project_config["cpp_compile_flag"].get<string>() +
                            " " + "-I" + include_dir;
  }

  // TODO pass as K-V args
  if (argc == 4) {
    if (argv[3] == string("0")) {
      // do nothing
    } else if (argv[3] == string("1")) {   // run with debug flags
      project_config["use_cache"] = false; // don't use cache for debuging
      compiler_flags = "cpp_debug_flag";
    } else if (argv[3] == string("2")) {
      /*
        Run with terminal. This option only uses the project config file for
        compiler flags. No problem config will be used.
      */
      solution_file_path =
          root_dir / "solution.cpp"; // NOTE support c++ for now
      check_file(solution_file_path, "solution file not found");
      compile_cpp(root_dir, false, solution_file_path,
                  project_config[compiler_flags], "solution");
      // copy solution file to output dir for submission
      copy_file(solution_file_path, output_dir / "solution.cpp",
                fs::copy_options::overwrite_existing);
      int status = system_wraper("./solution");
      cout << '\n'; // empty line before printing the status
      if (status != 0) {
        cout << termcolor::red << "[Process exited " << status << "]"
             << termcolor::reset << "\n";
      } else {
        cout << "[Process exited 0]\n";
      }
      clean_up();
      return 0;
    } else if (argv[3] == string("3")) { // edit config
      fs::path template_dir =
          fs::absolute(project_config["template_dir"].get<string>());
      fs::path frontend_path =
          fs::absolute(project_config["frontend_path"].get<string>());
      edit_config(root_dir, template_dir, frontend_path);
      return 0;
    } else if (argv[3] == string("4")) { // archive
      fs::path temp_config_path =
          fs::absolute(project_config["template_dir"].get<string>()) /
          "config.template";
      problem_config_path = root_dir / "config.json";
      config = read_problem_config(
          problem_config_path,
          temp_config_path); // reade the project config into a json object
      validate_problem_config(config);
      string name = config["name"].get<string>();
      string group = config["group"].get<string>();
      fs::current_path(root_dir.parent_path());

      fs::path archive_dir =
          fs::absolute(project_config["archive_dir"].get<string>());
      if (group.size() == 0) {
        fs::create_directories(archive_dir / "Unsorted" / name);
        fs::copy(name, archive_dir / "Unsorted" / name,
                 fs::copy_options::recursive |
                     fs::copy_options::update_existing);
      } else {
        fs::create_directories(archive_dir / group / name);
        fs::copy(name, archive_dir / group / name,
                 fs::copy_options::recursive |
                     fs::copy_options::update_existing);
      }
      fs::remove_all(name);
      return 0;
    } else {
      cout << termcolor::red << "[cpcli] unknown operation" << endl;
      return 0;
    }
  }

  binary_dir = cpcli_dir / "binary";
  check_dir(binary_dir, "binary_dir not found!");

  if (project_config["use_precompiled_header"]) {
    // TODO validate precompiled_header

    fs::path precompiled_dir = binary_dir / "precompiled_headers";

    fs::path precompiled_path =
        precompiled_dir / "cpp_compile_flag" / "stdc++.h";
    check_file(precompiled_dir / "cpp_compile_flag" / "stdc++.h.gch",
               "precompiled header not found!");

    fs::path precompiled_debug_path =
        precompiled_dir / "cpp_debug_flag" / "stdc++.h";
    check_file(precompiled_dir / "cpp_debug_flag" / "stdc++.h.gch",
               "precompiled debug header not found!");

    project_config["cpp_compile_flag"] =
        project_config["cpp_compile_flag"].get<string>() + " " + "-include" +
        " \"" + precompiled_path.string() + "\"";
    testlib_compiler_flag = testlib_compiler_flag + " " + "-include" + " \"" +
                            precompiled_path.string() + "\"";
    project_config["cpp_debug_flag"] =
        project_config["cpp_debug_flag"].get<string>() + " " + "-include" +
        " \"" + precompiled_debug_path.string() + "\"";
  }

  check_file(project_config_path,
             "project config file not found"); // check if the
                                               // project_config.json exists

  problem_config_path = root_dir / "config.json";

  // TODO check if template exists
  fs::path temp_config_path =
      fs::absolute(project_config["template_dir"].get<string>()) /
      "config.template";
  config = read_problem_config(
      problem_config_path,
      temp_config_path); // reade the project config into a json object
  validate_problem_config(config);

  if (config["group"] != nullptr && config["group"].get<string>().size() != 0) {
    cout << config["group"].get<string>() << '\n';
  }
  cout << config["name"].get<string>() << '\n';

  // ----------------------------- COMPILE START ----------------------------
  {
    auto t0 = std::chrono::high_resolution_clock::now();
    fs::path cache_dir = "";

    bool use_cache = project_config["use_cache"];
    if (use_cache) {
      cache_dir = fs::absolute("/tmp/cpcli") /
                  to_string(std::hash<std::string>()(root_dir));
      fs::create_directories(cache_dir);
    }

    if (config["interactive"]) {
      config["knowGenAns"] = false;
      fs::path interactor_file_path = root_dir / "interactor.cpp";
      check_file(interactor_file_path, "interactor file not found!");
      compile_cpp(cache_dir, use_cache, interactor_file_path,
                  project_config[compiler_flags], "interactor");
      cout << termcolor::cyan << termcolor::bold << "Interactive task"
           << termcolor::reset << '\n';
    } else {
      if (config["checker"] != "custom") {
        fs::path checker_bin_path = binary_dir / "checker" / config["checker"];
        check_file(checker_bin_path, "checker binary not found!");
        copy_file(checker_bin_path, root_dir / "checker",
                  fs::copy_options::overwrite_existing);
      } else {
        fs::path checker_file_path = root_dir / "checker.cpp";
        check_file(checker_file_path, "checker file not found!");
        compile_cpp(cache_dir, use_cache, checker_file_path,
                    testlib_compiler_flag, "checker");
      }
      cout << termcolor::cyan << termcolor::bold << "Using "
           << config["checker"].get<string>() << " checker!" << termcolor::reset
           << '\n';
    }

    // use slow solution for generate correct output
    // requre slow.cpp
    if (config["knowGenAns"]) {
      fs::path slow_file_path = root_dir / "slow.cpp";
      check_file(slow_file_path, "brute force solution file not found!");
      compile_cpp(cache_dir, use_cache, slow_file_path,
                  project_config[compiler_flags], "slow");
    }

    if (config["useGeneration"]) {
      fs::path gen_file_path = root_dir / "gen.cpp";
      check_file(gen_file_path, "gen file not found!");
      compile_cpp(cache_dir, use_cache, gen_file_path, testlib_compiler_flag,
                  "gen");
      generator_seed = config["generatorSeed"];
      if (config["generatorSeed"].get<string>().size() == 0) {
        generator_seed = gen_string_length_20();
      }
      cout << termcolor::yellow << termcolor::bold
           << "Stress testing with seed \'" << generator_seed << "\'"
           << termcolor::reset << '\n';
    }

    // compile solution file
    solution_file_path = root_dir / "solution.cpp"; // NOTE support c++ for now
    check_file(solution_file_path, "solution file not found");
    compile_cpp(cache_dir, use_cache, solution_file_path,
                project_config[compiler_flags], "solution");
    copy_file(
        solution_file_path, output_dir / "solution.cpp",
        fs::copy_options::overwrite_existing); // copy solution file to output
                                               // dir for submission

    auto t1 = std::chrono::high_resolution_clock::now();
    long long time =
        std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
    cout << termcolor::magenta << termcolor::bold << "Compilation finished in "
         << time << " ms" << endl;

    cout << DASH_SEPERATOR << '\n';
  }
  // ------------------------------ COMPILE END -----------------------------

  // ------------------------ GENERATING TESTS START ------------------------

  {
    fs::current_path(root_dir);
    mkdir("___test_case", 0777);
    fs::current_path("___test_case");
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
      string command =
          "../gen " + generator_seed + " " +
          to_string(config["numTest"].get<int>()); // NOTE careful with ..
      int status = system_wraper(command);
      if (status != 0) {
        cout << termcolor::red << termcolor::bold << "generator run time error"
             << termcolor::reset << endl;
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
    fs::current_path(root_dir);
    long long time_limit = config["timeLimit"].get<long long>();
    auto tests_folder_dir =
        fs::path("___test_case"); // set the root directory to argv[1]
    create_empty_file(tests_folder_dir / "___na___");

    std::vector<std::pair<int, fs::path>> sorted_by_name;
    for (auto &entry : fs::directory_iterator(tests_folder_dir)) {
      if (entry.path().extension() == ".in") {
        const auto test_id = entry.path().stem().string();
        int num = 0;
        if (test_id[0] == 'S') {
          num = 1000000000 +
                std::stoi(test_id.substr(
                    1)); // funny trick to ensure stress tests come after
        } else {
          num = std::stoi(test_id);
        }
        sorted_by_name.push_back({num, entry.path()});
      }
    }
    sort(sorted_by_name.begin(), sorted_by_name.end());

    for (const auto &set_entry : sorted_by_name) {
      auto entry = set_entry.second;
      bool passed = 1, undecided = 0, rte = 0, tle = 0, wa = 0;
      long long runtime = 0;
      const auto test_id = entry.stem().string();
      const auto actual_file = tests_folder_dir / (test_id + ".actual");
      const auto out_file = tests_folder_dir / (test_id + ".out");
      const auto res_file = tests_folder_dir / (test_id + ".res");
      bool truncate = config["truncateLongTest"].get<bool>();

      create_empty_file(res_file);

      // --------- test id ------------
      if (test_id[0] != 'S') {
        cout << termcolor::cyan << termcolor::bold << "Test #" << test_id
             << ": " << termcolor::reset;
      } else {
        cout << termcolor::yellow << termcolor::bold << "Test #" << test_id
             << ": " << termcolor::reset;
      }

      if (config["interactive"]) {
        string command = "./interactor " + entry.string() + " " +
                         actual_file.string() + " " + res_file.string();
        int status = system_wraper(command);
        if (status != 0) {
          passed = 0;
          if (status != 1) {
            rte = 1;
          }
        }
        wa = !passed && !undecided;
      } else {
        {
          string command =
              "./solution < " + entry.string() + " > " + actual_file.string();

          auto t0 = std::chrono::high_resolution_clock::now();
          int status = system_wraper(command);
          auto t1 = std::chrono::high_resolution_clock::now();
          runtime =
              std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0)
                  .count();
          if (runtime > time_limit) {
            tle = 1;
            passed = 0;
          }
          if (status != 0) {
            rte = 1;
            passed = 0;
          }
        }

        {
          if (config["knowGenAns"]) {
            string command =
                "./slow < " + entry.string() + " > " + out_file.string();
            int status = system_wraper(command);
            if (status != 0) {
              cout << "Input:" << '\n';
              print_file(entry.string(), truncate);
              cout.flush();
              cout << DASH_SEPERATOR << '\n';
              cout << termcolor::red << termcolor::bold
                   << "slow solution run time error" << termcolor::reset
                   << endl;
              clean_up();
              exit(1);
            }
          }
        }

        {
          string out_file_str = out_file.string();
          if (!check_file(out_file, "")) {
            out_file_str = tests_folder_dir / "___na___";
          }
          // ./checker <input> <pout> <jans> <res>
          string command = "./checker " + entry.string() + "  " +
                           actual_file.string() + " " + out_file_str + " " +
                           res_file.string() + " > /dev/null 2>&1";
          int status = system_wraper(command);
          if (status != 0) {
            passed = 0;
          }
          if (status == 3) {
            undecided = 1;
          }
          wa = !passed && !undecided && !tle && !rte;
        }
      }
      all_passed &= passed;
      all_rte |= rte;
      all_tle |= tle;
      all_wa |= wa;
      all_runtime = std::max(all_runtime, runtime);
      if (passed && config["hideAcceptedTest"]) {
        cout << termcolor::green << termcolor::bold << "accepted"
             << termcolor::reset << '\n';
      } else {
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
        if (wa) {
          print_file(res_file, false);
        }
      }
      cout << DASH_SEPERATOR << '\n';
      fs::current_path(root_dir);

      if (config["stopAtWrongAnswer"] && (wa || rte || tle)) {
        print_report("Fail detected", all_passed, all_rte, all_tle, all_wa,
                     all_runtime);
        clean_up();
        return 0;
      }
    }
  }
  // ------------------------------ TESTS END -------------------------------

  // ------------------------------ PRINT REPORT START
  // -------------------------------
  {
    cout << EQUA_SEPERATOR << '\n';
    print_report("Results", all_passed, all_rte, all_tle, all_wa, all_runtime);
    clean_up();
  }
  // ------------------------------ PRINT REPORT END
  // -------------------------------
}
