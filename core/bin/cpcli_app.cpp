#include "color.hpp"
#include "compiler.hpp"
#include "constant.hpp"
#include "nlohmann/json.hpp"
#include "operations.hpp"
#include "path_manager.hpp"
#include "spdlog/spdlog.h"
#include "template_manager.hpp"
#include "utils.hpp"

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <signal.h>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

using std::cout;
using std::endl;
using std::string;
using json = nlohmann::json;

namespace {
struct TestResult {
  bool passed = true;
  bool rte = false;
  bool tle = false;
  bool wa = false;
  long long runtime = 0;
};

void merge_result(TestResult &total, const TestResult &test) {
  total.passed &= test.passed;
  total.rte |= test.rte;
  total.tle |= test.tle;
  total.wa |= test.wa;
  total.runtime = std::max(total.runtime, test.runtime);
}

string shell_quote(const std::filesystem::path &path) {
  string value = path.string();
  string quoted = "'";
  for (const char ch : value) {
    if (ch == '\'') {
      quoted += "'\\''";
    } else {
      quoted += ch;
    }
  }
  return quoted + "'";
}

string subtask_label(const json &subtask) {
  const auto name = subtask.value("name", string());
  return name.empty() ? "default" : name;
}
} // namespace

int cpcli_process(int argc, char *argv[]) {
  signal(SIGINT, [](int) { handle_sigint(); });
  auto parser_result = parse_args(argc, argv);
  auto t_start = std::chrono::high_resolution_clock::now();

  const std::filesystem::path project_conf_path = parser_result.project_config_path;
  json project_conf = read_project_config(project_conf_path);

  PathManager path_manager;
  auto status = path_manager.init(project_conf);
  if (status != PathManagerStatus::Success) {
    spdlog::error("Path manager return non success code. Exiting...");
    exit(PathManagerFailToInitFromConfig);
  }
  TemplateManager template_manager(path_manager, project_conf);

  const auto local_share_dir = path_manager.get_local_share();
  spdlog::debug("local_share_dir directory is: " + local_share_dir.string());
  const auto checker_dir = local_share_dir / "checkers";

  if (parser_result.operation == ParserOperations::NewTask) {
    create_new_task(project_conf_path);
    return 0;
  }

  const std::filesystem::path root_dir = *parser_result.root_dir;
  std::filesystem::current_path(root_dir);
  clean_up();

  if (parser_result.operation == ParserOperations::EditTaskConfig) {
    string task_editor_exec = project_conf["task_editor_exec"].get<string>();
    edit_config(root_dir, project_conf_path, template_manager, task_editor_exec);
    return 0;
  }

  const auto problem_conf_path = resolve_problem_config_path(root_dir);
  json problem_conf = read_problem_config(problem_conf_path, template_manager.get_problem_config());
  path_manager.init_problem_conf(problem_conf);

  bool is_debug = false;
  if (parser_result.operation == ParserOperations::Build) {
  } else if (parser_result.operation == ParserOperations::BuildWithDebug) {
    is_debug = true;
  } else if (parser_result.operation == ParserOperations::BuildWithTerm) {
    Compiler compiler(project_conf, path_manager, false);
    compiler.compile(path_manager.get_solution_path(root_dir));
    int process_status = system_warper("./solution");
    cout << '\n';
    if (process_status != 0) {
      cout << termcolor::red << "[Process exited " << process_status << "]" << termcolor::reset << "\n";
    } else {
      cout << "[Process exited 0]\n";
    }
    clean_up();
    print_duration(t_start);
    return 0;
  } else if (parser_result.operation == ParserOperations::Archive) {
    string name = problem_conf["name"].get<string>();
    string group = problem_conf["group"].get<string>();
    std::filesystem::current_path(root_dir.parent_path());
    const auto archive_dir = path_manager.get_archive();
    if (group.empty()) {
      group = "Unsorted";
    }
    std::filesystem::create_directories(archive_dir / group / name);
    std::filesystem::copy(name,
                          archive_dir / group / name,
                          std::filesystem::copy_options::recursive | std::filesystem::copy_options::update_existing);
    std::filesystem::remove_all(name);
    return 0;
  } else {
    cout << termcolor::red << "[cpcli] unknown operation" << endl;
    return OPERATION_ERR;
  }

  if (problem_conf["group"] != nullptr && !problem_conf["group"].get<string>().empty()) {
    cout << problem_conf["group"].get<string>() << '\n';
  }
  cout << problem_conf["name"].get<string>() << '\n';

  std::vector<json> subtasks;
  std::set<string> names;
  bool selected_subtask_found = false;
  for (const auto &subtask : problem_conf["subtasks"]) {
    const auto name = subtask.value("name", string());
    if (!names.insert(name).second) {
      cout << termcolor::red << "Duplicate subtask name: " << name << termcolor::reset << '\n';
      return INVALID_CONFIG_ERROR;
    }
    if (parser_result.subtask) {
      if (name == *parser_result.subtask) {
        subtasks.push_back(subtask);
        selected_subtask_found = true;
      }
    } else if (subtask.value("enabled", true)) {
      subtasks.push_back(subtask);
    }
  }
  if (parser_result.subtask && !selected_subtask_found) {
    cout << termcolor::red << "Unknown subtask: " << *parser_result.subtask << termcolor::reset << '\n';
    return INVALID_CONFIG_ERROR;
  }
  if (subtasks.empty()) {
    cout << termcolor::yellow << "No subtasks are enabled" << termcolor::reset << '\n';
    return 0;
  }
  for (const auto &subtask : problem_conf["subtasks"]) {
    for (const auto &dependency : subtask.value("dependsOn", json::array())) {
      const auto dependency_name = dependency.get<string>();
      if (names.find(dependency_name) == names.end()) {
        cout << termcolor::red << "Subtask " << subtask_label(subtask) << " depends on unknown subtask '"
             << dependency_name << "'" << termcolor::reset << '\n';
        return INVALID_CONFIG_ERROR;
      }
    }
  }
  for (const auto &test : problem_conf["tests"]) {
    if (test.value("subtaskIndex", -1) < 0) {
      cout << termcolor::red << "Test #" << test.value("index", -1) << " refers to unknown subtask '"
           << test.value("subtask", string()) << "'" << termcolor::reset << '\n';
      return INVALID_CONFIG_ERROR;
    }
  }

  std::map<size_t, std::filesystem::path> generator_executables;
  std::map<size_t, std::filesystem::path> slow_executables;
  std::map<size_t, std::filesystem::path> checker_executables;
  std::map<size_t, string> generator_seeds;
  std::set<std::filesystem::path> compiled_artifacts;
  auto cleanup_task = [&]() {
    std::filesystem::current_path(root_dir);
    for (const auto &artifact : compiled_artifacts) {
      std::filesystem::remove(artifact);
    }
    clean_up();
  };

  Compiler compiler(project_conf, path_manager, is_debug);
  {
    auto t0 = std::chrono::high_resolution_clock::now();
    std::set<std::filesystem::path> compiled_sources;
    auto compile_once = [&](const std::filesystem::path &source) {
      const auto resolved = std::filesystem::weakly_canonical(source);
      if (compiled_sources.insert(resolved).second) {
        compiler.compile(source);
      }
      const auto executable = root_dir / source.stem();
      compiled_artifacts.insert(executable);
      return executable;
    };

    if (problem_conf["interactive"]) {
      compile_once(path_manager.get_interactor_path(root_dir));
      cout << termcolor::cyan << termcolor::bold << "Interactive task" << termcolor::reset << '\n';
    }

    for (const auto &subtask : subtasks) {
      const auto index = subtask["index"].get<size_t>();
      if (!problem_conf["interactive"]) {
        const auto checker = subtask.value("checker", string("token_checker"));
        if (checker == "custom") {
          checker_executables[index] = compile_once(path_manager.get_checker_path(root_dir));
        } else {
          const auto checker_path = checker_dir / checker;
          check_file(checker_path, "checker binary not found!");
          checker_executables[index] = checker_path;
        }
        cout << termcolor::cyan << termcolor::bold << "Subtask " << subtask_label(subtask) << ": using " << checker
             << " checker" << termcolor::reset << '\n';
      }
      if (!problem_conf["interactive"] && subtask.value("knowGenAns", false)) {
        const auto source = path_manager.get_slow_path(root_dir, subtask.value("slow", string("slow")));
        slow_executables[index] = compile_once(source);
      }
      if (subtask.value("useGeneration", false)) {
        const auto source = path_manager.get_task_gen_path(root_dir, subtask.value("gen", string("gen")));
        generator_executables[index] = compile_once(source);
        auto seed = subtask.value("generatorSeed", string());
        if (seed.empty()) {
          seed = gen_string_length_20();
        }
        generator_seeds[index] = seed;
        cout << termcolor::yellow << termcolor::bold << "Subtask " << subtask_label(subtask)
             << ": stress testing with seed '" << seed << "'" << termcolor::reset << '\n';
      }
    }

    compiler.compile(path_manager.get_solution_path(root_dir));
    auto t1 = std::chrono::high_resolution_clock::now();
    const auto time = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
    cout << termcolor::magenta << termcolor::bold << "Compilation finished in " << time << " ms" << endl;
    cout << DASH_SEPERATOR << '\n';
  }

  {
    std::filesystem::current_path(root_dir);
    std::filesystem::create_directories("___test_case");
    std::set<size_t> selected_indices;
    for (const auto &subtask : subtasks) {
      selected_indices.insert(subtask["index"].get<size_t>());
    }
    for (const auto &test : problem_conf["tests"]) {
      const auto subtask_index = test["subtaskIndex"].get<size_t>();
      if (!test["active"] || selected_indices.find(subtask_index) == selected_indices.end()) {
        continue;
      }
      const auto test_dir = std::filesystem::path("___test_case") / std::to_string(subtask_index);
      std::filesystem::create_directories(test_dir);
      if (test["input"] != nullptr) {
        std::ofstream(test_dir / (std::to_string(test["index"].get<long long>()) + ".in"))
            << test["input"].get<string>();
      }
      if (test["answer"].get<bool>()) {
        std::ofstream(test_dir / (std::to_string(test["index"].get<long long>()) + ".out"))
            << test["output"].get<string>();
      }
    }

    for (const auto &subtask : subtasks) {
      if (!subtask.value("useGeneration", false)) {
        continue;
      }
      const auto index = subtask["index"].get<size_t>();
      const auto test_dir = root_dir / "___test_case" / std::to_string(index);
      std::filesystem::create_directories(test_dir);
      std::filesystem::current_path(test_dir);
      string command = shell_quote(std::filesystem::path("../..") / generator_executables[index].filename()) + " " +
                       generator_seeds[index] + " " + std::to_string(subtask.value("numTest", 0));
      const auto parameters = subtask.value("genParameters", string());
      if (!parameters.empty()) {
        command += " " + parameters;
      }
      if (system_warper(command) != 0) {
        cout << termcolor::red << termcolor::bold << "Subtask " << subtask_label(subtask) << " generator run time error"
             << termcolor::reset << endl;
        cleanup_task();
        return 1;
      }
    }
  }

  TestResult all_results;
  std::vector<TestResult> subtask_results;
  std::filesystem::current_path(root_dir);
  for (const auto &subtask : subtasks) {
    TestResult subtask_result;
    const auto subtask_index = subtask["index"].get<size_t>();
    const auto time_limit = subtask.value("timeLimit", 10000LL);
    const auto tests_folder_dir = std::filesystem::path("___test_case") / std::to_string(subtask_index);
    std::filesystem::create_directories(tests_folder_dir);
    cout << termcolor::cyan << termcolor::bold << "Subtask " << subtask_label(subtask) << termcolor::reset << '\n';

    std::vector<std::pair<int, std::filesystem::path>> sorted_by_name;
    for (const auto &entry : std::filesystem::directory_iterator(tests_folder_dir)) {
      if (entry.path().extension() != ".in") {
        continue;
      }
      const auto test_id = entry.path().stem().string();
      const auto order = test_id[0] == 'S' ? 1000000000 + std::stoi(test_id.substr(1)) : std::stoi(test_id);
      sorted_by_name.push_back({order, entry.path()});
    }
    std::sort(sorted_by_name.begin(), sorted_by_name.end());

    for (const auto &[_, entry] : sorted_by_name) {
      TestResult test_result;
      bool undecided = false;
      const auto test_id = entry.stem().string();
      const auto actual_file = tests_folder_dir / (test_id + ".actual");
      const auto out_file = tests_folder_dir / (test_id + ".out");
      const auto res_file = tests_folder_dir / (test_id + ".res");
      const bool truncate = problem_conf["truncateLongTest"].get<bool>();
      create_empty_file(res_file);

      if (test_id[0] == 'S') {
        cout << termcolor::yellow << termcolor::bold << "Test #" << test_id << ": " << termcolor::reset;
      } else {
        cout << termcolor::cyan << termcolor::bold << "Test #" << test_id << ": " << termcolor::reset;
      }

      if (problem_conf["interactive"]) {
        const string command =
            "./interactor " + shell_quote(entry) + " " + shell_quote(actual_file) + " " + shell_quote(res_file);
        const int process_status = system_warper(command);
        if (process_status != 0) {
          test_result.passed = false;
          test_result.rte = process_status != 1;
        }
        test_result.wa = !test_result.passed && !test_result.rte;
      } else {
        const string solution_command = "./solution < " + shell_quote(entry) + " > " + shell_quote(actual_file);
        auto t0 = std::chrono::high_resolution_clock::now();
        const int solution_status = system_warper(solution_command);
        auto t1 = std::chrono::high_resolution_clock::now();
        test_result.runtime = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
        if (test_result.runtime > time_limit) {
          test_result.tle = true;
          test_result.passed = false;
        }
        if (solution_status != 0) {
          test_result.rte = true;
          test_result.passed = false;
        }

        if (subtask.value("knowGenAns", false)) {
          const string slow_command =
              shell_quote(slow_executables[subtask_index]) + " < " + shell_quote(entry) + " > " + shell_quote(out_file);
          if (system_warper(slow_command) != 0) {
            cout << termcolor::red << termcolor::bold << "slow solution run time error" << termcolor::reset << endl;
            cleanup_task();
            return 1;
          }
        }

        if (!check_file(out_file, "")) {
          undecided = true;
          test_result.passed = false;
        } else {
          const string checker_command = shell_quote(checker_executables[subtask_index]) + " " + shell_quote(entry) +
                                         " " + shell_quote(actual_file) + " " + shell_quote(out_file) + " " +
                                         shell_quote(res_file) + " > /dev/null 2>&1";
          test_result.passed = system_warper(checker_command) == 0;
        }
        test_result.wa = !test_result.passed && !undecided && !test_result.tle && !test_result.rte;
      }

      merge_result(subtask_result, test_result);
      merge_result(all_results, test_result);
      if (test_result.passed && problem_conf["hideAcceptedTest"]) {
        cout << termcolor::green << termcolor::bold << "accepted" << termcolor::reset << '\n';
      } else {
        cout << '\n' << "Input:" << '\n';
        print_file(entry.string(), truncate);
        if (check_file(out_file, "")) {
          cout << "Expected output:" << '\n';
          print_file(out_file, truncate);
        }
        cout << "Execution output:" << '\n';
        print_file(actual_file, truncate);
        print_report(
            "Verdict", test_result.passed, test_result.rte, test_result.tle, test_result.wa, test_result.runtime);
        if (!is_empty_file(res_file.string())) {
          print_file(res_file, false);
        }
      }
      cout << DASH_SEPERATOR << '\n';

      if (problem_conf["stopAtWrongAnswer"] && (test_result.wa || test_result.rte || test_result.tle)) {
        print_report(
            "Fail detected", all_results.passed, all_results.rte, all_results.tle, all_results.wa, all_results.runtime);
        cleanup_task();
        print_duration(t_start);
        return 0;
      }
    }
    subtask_results.push_back(subtask_result);
  }

  cout << EQUA_SEPERATOR << '\n';
  std::map<string, bool> passed_by_name;
  for (size_t i = 0; i < subtasks.size(); ++i) {
    passed_by_name[subtasks[i].value("name", string())] = subtask_results[i].passed;
    print_report("Subtask " + subtask_label(subtasks[i]),
                 subtask_results[i].passed,
                 subtask_results[i].rte,
                 subtask_results[i].tle,
                 subtask_results[i].wa,
                 subtask_results[i].runtime);
  }
  print_report("Results", all_results.passed, all_results.rte, all_results.tle, all_results.wa, all_results.runtime);

  long long earned_points = 0;
  long long available_points = 0;
  bool has_points = false;
  for (size_t i = 0; i < subtasks.size(); ++i) {
    if (!subtasks[i].contains("points") || subtasks[i]["points"].is_null()) {
      continue;
    }
    has_points = true;
    const auto points = subtasks[i]["points"].get<long long>();
    available_points += points;
    bool dependencies_passed = true;
    for (const auto &dependency : subtasks[i].value("dependsOn", json::array())) {
      const auto dependency_result = passed_by_name.find(dependency.get<string>());
      dependencies_passed &= dependency_result != passed_by_name.end() && dependency_result->second;
    }
    if (subtask_results[i].passed && dependencies_passed) {
      earned_points += points;
    }
  }
  if (has_points && parser_result.subtask) {
    cout << "Score: not calculated for --subtask runs\n";
  } else if (has_points) {
    cout << "Score: " << earned_points << "/" << available_points << '\n';
  }

  cleanup_task();
  print_duration(t_start);
  return 0;
}

int main(int argc, char *argv[]) { return cpcli_process(argc, argv); }
