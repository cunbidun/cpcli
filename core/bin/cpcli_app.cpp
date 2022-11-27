#include <algorithm>
#include <filesystem>
#include <fstream>
#include <getopt.h>
#include <iostream>
#include <set>
#include <signal.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "color.hpp"
#include "constant.hpp"
#include "nlohmann/json.hpp"
#include "operations.hpp"
#include "path_manager.hpp"
#include "spdlog/spdlog.h"
#include "template_manager.hpp"
#include "utils.hpp"

using std::cout;
using std::endl;
using std::string;
using json = nlohmann::json;

int cpcli_process(int argc, char *argv[]) {
  signal(SIGINT, [](int) { handle_sigint(); }); // implement SIGINT
  auto parser_result = parse_args(argc, argv);
  std::chrono::high_resolution_clock::time_point t_start = std::chrono::high_resolution_clock::now();

  // project_config's path and project_config json object
  std::filesystem::path project_conf_path = parser_result.project_config_path;
  json project_conf = read_project_config(project_conf_path);

  std::filesystem::path root_dir;   // where source files and problem_config file located
  std::filesystem::path output_dir; // where source will be put for submission

  // problem's config path and config json object
  std::filesystem::path problem_conf_path;
  json problem_conf;

  std::filesystem::path solution_file_path;

  string testlib_compiler_flag;
  string compiler_flags = "cpp_compile_flag";
  string generator_seed;

  PathManager path_manager;
  auto status = path_manager.init(project_conf);
  if (status != PathManagerStatus::Success) {
    spdlog::error("Path manager return non success code. Exiting...");
    exit(PathManagerFailToInitFromConfig);
  }
  TemplateManager template_manager(path_manager, "cpp", project_conf.value("use_template_engine", false));

  std::filesystem::path local_share_dir = path_manager.get_local_share();
  spdlog::debug("local_share_dir directory is: " + local_share_dir.string());
  std::filesystem::path checker_dir = local_share_dir / "checkers";
  std::filesystem::path precompiled_dir = local_share_dir / "precompiled_headers";

  if (parser_result.operation == ParserOperations::NewTask) {
    create_new_task(project_conf);
    return 0;
  }

  if (parser_result.operation == ParserOperations::GenHeader) {
    spdlog::debug("Generating precompiled headers");
    spdlog::debug("precompiled_dir is: " + precompiled_dir.string());
    spdlog::debug("cpp_compiler is: " + project_conf["cpp_compiler"].get<string>());
    spdlog::debug("cpp_compile_flag is: " + project_conf["cpp_compile_flag"].get<string>());
    spdlog::debug("cpp_debug_flag is: " + project_conf["cpp_debug_flag"].get<string>());
    compile_headers(precompiled_dir,
                    project_conf["cpp_compiler"].get<string>(),
                    project_conf["cpp_compile_flag"].get<string>(),
                    project_conf["cpp_debug_flag"].get<string>());
    return 0;
  }

  root_dir = *parser_result.root_dir;
  std::filesystem::current_path(root_dir); // change directory to root_dir
  clean_up();                 // clean up the root directory for the first time

  if (parser_result.operation == ParserOperations::EditTaskConfig) { // edit config
    string frontend_exec = project_conf["frontend_exec"].get<string>();
    edit_config(root_dir, template_manager, frontend_exec);
    return 0;
  }

  std::filesystem::path temp_config_path = template_manager.get_problem_config();
  problem_conf_path = root_dir / "config.json";
  problem_conf = read_problem_config(problem_conf_path, temp_config_path);

  output_dir = path_manager.get_output();

  testlib_compiler_flag = project_conf["cpp_compile_flag"].get<string>();
  if (path_manager.has_customize_include_dir()) {
    string include_dir = "\"" + path_manager.get_include().generic_string() + "\"";
    std::vector<string> command{project_conf["cpp_compile_flag"].get<string>(), "-I", include_dir};
    testlib_compiler_flag = join(command);
  }

  if (parser_result.operation == ParserOperations::Build) {
    // do nothing
  } else if (parser_result.operation == ParserOperations::BuildWithDebug) { // run with debug flags
    project_conf["use_cache"] = false;                                      // don't use cache for debugging
    compiler_flags = "cpp_debug_flag";
  } else if (parser_result.operation == ParserOperations::BuildWithTerm) {
    // Run with terminal. This option only uses the project config file for
    // compiler flags. No problem config will be used.
    solution_file_path = root_dir / "solution.cpp"; // NOTE support c++ for now
    check_file(solution_file_path, "solution file not found");
    compile_cpp(
        root_dir, false, project_conf["cpp_compiler"], solution_file_path, project_conf[compiler_flags], "solution");
    // copy solution file to output dir for submission
    copy_file(solution_file_path, output_dir / "solution.cpp", std::filesystem::copy_options::overwrite_existing);
    int status = system_warper("./solution");
    cout << '\n'; // add an empty line before printing the status
    if (status != 0) {
      cout << termcolor::red << "[Process exited " << status << "]" << termcolor::reset << "\n";
    } else {
      cout << "[Process exited 0]\n";
    }
    clean_up();
    print_duration(t_start);
    return 0;
  } else if (parser_result.operation == ParserOperations::Archive) { // archive
    string name = problem_conf["name"].get<string>();
    string group = problem_conf["group"].get<string>();

    // We move back to the parent directory of current task in order to copy it
    std::filesystem::current_path(root_dir.parent_path());

    auto archive_dir = path_manager.get_archive();
    if (group.empty()) {
      group = "Unsorted";
    }
    std::filesystem::create_directories(archive_dir / group / name);
    std::filesystem::copy(name, archive_dir / group / name, std::filesystem::copy_options::recursive | std::filesystem::copy_options::update_existing);
    std::filesystem::remove_all(name);
    return 0;
  } else {
    cout << termcolor::red << "[cpcli] unknown operation" << endl;
    return OPERATION_ERR;
  }

  if (project_conf["use_precompiled_header"]) {
    std::filesystem::path precompiled_path = precompiled_dir / "cpp_compile_flag" / "stdc++.h";
    check_file(precompiled_dir / "cpp_compile_flag" / "stdc++.h.gch",
               "precompiled header not found! Please try 'cpcli_app project -g'");

    std::filesystem::path precompiled_debug_path = precompiled_dir / "cpp_debug_flag" / "stdc++.h";
    check_file(precompiled_dir / "cpp_debug_flag" / "stdc++.h.gch",
               "precompiled debug header not found! Please try 'cpcli_app project -g'");

    project_conf["cpp_compile_flag"] =
        project_conf["cpp_compile_flag"].get<string>() + " " + "-include" + " \"" + precompiled_path.string() + "\"";
    testlib_compiler_flag = testlib_compiler_flag + " " + "-include" + " \"" + precompiled_path.string() + "\"";
    project_conf["cpp_debug_flag"] = project_conf["cpp_debug_flag"].get<string>() + " " + "-include" + " \"" +
                                     precompiled_debug_path.string() + "\"";
  }

  if (problem_conf["group"] != nullptr && problem_conf["group"].get<string>().size() != 0) {
    cout << problem_conf["group"].get<string>() << '\n';
  }
  cout << problem_conf["name"].get<string>() << '\n';

  // ----------------------------- COMPILE START ----------------------------
  {
    auto t0 = std::chrono::high_resolution_clock::now();
    std::filesystem::path cache_dir = "";

    bool use_cache = project_conf["use_cache"];
    if (use_cache) {
      cache_dir = std::filesystem::temp_directory_path() / "cpcli" / std::to_string(std::hash<std::string>()(root_dir));
      std::filesystem::create_directories(cache_dir);
    }

    if (problem_conf["interactive"]) {
      problem_conf["knowGenAns"] = false;
      std::filesystem::path interactor_file_path = root_dir / "interactor.cpp";
      check_file(interactor_file_path, "interactor file not found!");
      compile_cpp(cache_dir,
                  use_cache,
                  project_conf["cpp_compiler"],
                  interactor_file_path,
                  testlib_compiler_flag,
                  "interactor");
      cout << termcolor::cyan << termcolor::bold << "Interactive task" << termcolor::reset << '\n';
    } else {
      if (problem_conf["checker"] != "custom") {
        std::filesystem::path checker_bin_path = checker_dir / problem_conf["checker"];
        check_file(checker_bin_path, "checker binary not found!");
        copy_file(checker_bin_path, root_dir / "checker", std::filesystem::copy_options::overwrite_existing);
      } else {
        std::filesystem::path checker_file_path = root_dir / "checker.cpp";
        check_file(checker_file_path, "checker file not found!");
        compile_cpp(
            cache_dir, use_cache, project_conf["cpp_compiler"], checker_file_path, testlib_compiler_flag, "checker");
      }
      cout << termcolor::cyan << termcolor::bold << "Using " << problem_conf["checker"].get<string>() << " checker!"
           << termcolor::reset << '\n';
    }

    // use slow solution for generate correct output
    // require slow.cpp
    if (problem_conf["knowGenAns"]) {
      std::filesystem::path slow_file_path = root_dir / "slow.cpp";
      check_file(slow_file_path, "brute force solution file not found!");
      compile_cpp(
          cache_dir, use_cache, project_conf["cpp_compiler"], slow_file_path, project_conf[compiler_flags], "slow");
    }

    if (problem_conf["useGeneration"]) {
      std::filesystem::path gen_file_path = root_dir / "gen.cpp";
      check_file(gen_file_path, "gen file not found!");
      compile_cpp(cache_dir, use_cache, project_conf["cpp_compiler"], gen_file_path, testlib_compiler_flag, "gen");
      generator_seed = problem_conf["generatorSeed"];
      if (problem_conf["generatorSeed"].get<string>().size() == 0) {
        generator_seed = gen_string_length_20();
      }
      cout << termcolor::yellow << termcolor::bold << "Stress testing with seed \'" << generator_seed << "\'"
           << termcolor::reset << '\n';
    }

    // compile solution file
    solution_file_path = root_dir / "solution.cpp"; // NOTE support c++ for now
    check_file(solution_file_path, "solution file not found");
    compile_cpp(cache_dir,
                use_cache,
                project_conf["cpp_compiler"],
                solution_file_path,
                project_conf[compiler_flags],
                "solution");
    copy_file(solution_file_path,
              output_dir / "solution.cpp",
              std::filesystem::copy_options::overwrite_existing); // copy solution file to output
                                                     // dir for submission

    auto t1 = std::chrono::high_resolution_clock::now();
    long long time = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
    cout << termcolor::magenta << termcolor::bold << "Compilation finished in " << time << " ms" << endl;

    cout << DASH_SEPERATOR << '\n';
  }
  // ------------------------------ COMPILE END -----------------------------

  // ------------------------ GENERATING TESTS START ------------------------
  {
    std::filesystem::current_path(root_dir);
    std::filesystem::create_directory("___test_case");
    std::filesystem::current_path("___test_case");
    for (json test : problem_conf["tests"]) {
      if (test["active"]) {
        if (test["input"] != nullptr) {
          std::ofstream inf(test["index"].get<string>() + ".in");
          inf << test["input"].get<string>();
          inf.close();
        }
        if (test["answer"].get<bool>()) {
          std::ofstream ouf(test["index"].get<string>() + ".out");
          ouf << test["output"].get<string>();
          ouf.close();
        }
      }
    }

    if (problem_conf["useGeneration"]) {
      string command =
          "../gen " + generator_seed + " " + std::to_string(problem_conf["numTest"].get<int>()); // NOTE careful with ..
      int status = system_warper(command);
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
    long long time_limit = problem_conf["timeLimit"].get<long long>();
    auto tests_folder_dir = std::filesystem::path("___test_case"); // set the root directory to argv[1]

    std::vector<std::pair<int, std::filesystem::path>> sorted_by_name;
    for (auto &entry : std::filesystem::directory_iterator(tests_folder_dir)) {
      if (entry.path().extension() == ".in") {
        const auto test_id = entry.path().stem().string();
        int num = 0;
        if (test_id[0] == 'S') {
          num = 1000000000 + std::stoi(test_id.substr(1)); // funny trick to ensure stress tests come after
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
      bool truncate = problem_conf["truncateLongTest"].get<bool>();

      create_empty_file(res_file);

      // --------- test id ------------
      if (test_id[0] != 'S') {
        cout << termcolor::cyan << termcolor::bold << "Test #" << test_id << ": " << termcolor::reset;
      } else {
        cout << termcolor::yellow << termcolor::bold << "Test #" << test_id << ": " << termcolor::reset;
      }

      if (problem_conf["interactive"]) {
        string command = "./interactor " + entry.string() + " " + actual_file.string() + " " + res_file.string();
        int status = system_warper(command);
        if (status != 0) {
          passed = 0;
          if (status != 1) {
            rte = 1;
          }
        }
        wa = !passed && !undecided;
      } else {
        {
          string command = "./solution < " + entry.string() + " > " + actual_file.string();

          auto t0 = std::chrono::high_resolution_clock::now();
          int status = system_warper(command);
          auto t1 = std::chrono::high_resolution_clock::now();
          runtime = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
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
          if (problem_conf["knowGenAns"]) {
            string command = "./slow < " + entry.string() + " > " + out_file.string();
            int status = system_warper(command);
            if (status != 0) {
              cout << "Input:" << '\n';
              print_file(entry.string(), truncate);
              cout.flush();
              cout << DASH_SEPERATOR << '\n';
              cout << termcolor::red << termcolor::bold << "slow solution run time error" << termcolor::reset << endl;
              clean_up();
              exit(1);
            }
          }
        }

        {
          if (!check_file(out_file, "")) {
            undecided = 1;
            passed = 0;
          } else {
            string jans_file = out_file.string();
            // ./checker <input> <pout> <jans_file> <res>
            string command = "./checker " + entry.string() + "  " + actual_file.string() + " " + jans_file + " " +
                             res_file.string() + " > /dev/null 2>&1";
            passed = system_warper(command) == 0;
          }
          wa = !passed && !undecided && !tle && !rte;
        }
      }
      all_passed &= passed;
      all_rte |= rte;
      all_tle |= tle;
      all_wa |= wa;
      all_runtime = std::max(all_runtime, runtime);
      if (passed && problem_conf["hideAcceptedTest"]) {
        cout << termcolor::green << termcolor::bold << "accepted" << termcolor::reset << '\n';
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
        if (!is_empty_file(res_file.string())) {
          print_file(res_file, false);
        }
      }
      cout << DASH_SEPERATOR << '\n';
      std::filesystem::current_path(root_dir);

      if (problem_conf["stopAtWrongAnswer"] && (wa || rte || tle)) {
        print_report("Fail detected", all_passed, all_rte, all_tle, all_wa, all_runtime);
        clean_up();
        print_duration(t_start);
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
    print_duration(t_start);
  }
  // ------------------------------ PRINT REPORT END -------------------------------
  return 0;
}

int main(int argc, char *argv[]) { return cpcli_process(argc, argv); }