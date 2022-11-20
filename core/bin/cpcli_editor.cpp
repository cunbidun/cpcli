#include "CLI/CLI.hpp"
#include "color.hpp"
#include "constant.hpp"
#include "nlohmann/json.hpp"
#include "spdlog/spdlog.h"
#include "utils.hpp"

#include <filesystem>
#include <fstream>
#include <vector>

namespace fs = std::filesystem;
using json = nlohmann::json;
using std::cout;
using std::vector;

int print_tests(vector<int> &test_index_vector, json test_data) {
  sort(test_index_vector.begin(), test_index_vector.end());
  for (auto i : test_index_vector) {
    cout << DASH_SEPERATOR << std::endl;
    auto data = test_data["tests"][i];
    spdlog::debug("Printing test {}. Raw test data={}", i, data.dump(4));

    cout << "Test #" << data["index"] << ": ";
    if (data["active"].get<bool>()) {
      cout << termcolor::green << termcolor::bold << "active" << termcolor::reset;
    } else {
      cout << termcolor::red << termcolor::bold << "inactive" << termcolor::reset;
    }
    cout << std::endl;
    cout << "Input:" << '\n' << data["input"].get<std::string>() << std::endl;
    cout << "Expected output:" << '\n';
    if (data["output"].is_null()) {
      cout << termcolor::yellow << termcolor::bold;
      cout << "no expected output" << std::endl;
    } else {
      cout << data["output"].get<std::string>() << std::endl;
    }
  }
  return 0;
}

int main(int argc, char *argv[]) {

  CLI::App parser{"Task Editor CLI For cpcli"};

  fs::path config_path;
  std::vector<int> test_index_vector;
  int test_index = -1;
  bool add_not_active = false;
  bool add_not_know_ans = false;
  string checker_name;
  int time_limit;
  int generator_num_test;
  string generator_seed;
  string generator_params;
  string task_name;
  string task_group;

  parser.set_version_flag("-v,--version", VERSION);
  parser.set_help_all_flag("-H,--help-all", "Print subcommand help and exit");

  parser.add_flag_function(
      "-d,--debug",
      [](int count) {
        spdlog::set_level(spdlog::level::debug);
        spdlog::debug("Debug (--debug) is set");
      },
      "Run with debug flags (this option will print debug logs)");
  parser.add_option("-p,--problem-config", config_path, "Path to the problem config file")
      ->required(true)
      ->check(CLI::ExistingFile)
      ->transform([](std::filesystem::path path) { return std::filesystem::canonical(path); });

  /**
   * operation on task's test cases
   */
  auto test = parser.add_subcommand("test", "Operation on task's test cases");
  auto print = test->add_subcommand("print", "Print tests");
  print->add_option<std::vector<int>>("nums", test_index_vector, "Print one multiple tests")
      ->check(CLI::NonNegativeNumber);
  auto all = test->add_subcommand("all", "Select all tests");
  all->add_option<std::vector<int>>("nums", test_index_vector, "Select one multiple tests")
      ->check(CLI::NonNegativeNumber);
  auto none = test->add_subcommand("none", "Select none tests");
  none->add_option<std::vector<int>>("nums", test_index_vector, "Unselect one multiple tests")
      ->check(CLI::NonNegativeNumber);

  auto add = test->add_subcommand("add", "Add a test");
  add->add_option<int>("num", test_index, "Test index to clone")->check(CLI::NonNegativeNumber);
  add->add_flag("-i,--inactive", add_not_active, "Set to true if test is inactive")->default_val(false);
  add->add_flag("-u,--unknown", add_not_know_ans, "Set to true if don't know test correct output")->default_val(false);

  auto del = test->add_subcommand("del", "Delete a test");
  del->add_option<int>("num", test_index, "Test index to delete")->check(CLI::NonNegativeNumber)->required(true);

  auto edit = test->add_subcommand("edit", "Edit a test");
  edit->add_option<int>("num", test_index, "Test index to edit")->check(CLI::NonNegativeNumber)->required(true);

  auto unknow = test->add_subcommand("unknow", "Mark a test expected output as unknown");
  unknow->add_option<int>("num", test_index, "Test index to edit")->check(CLI::NonNegativeNumber)->required(true);
  test->require_subcommand(1);

  /**
   * operation on task's option
   */
  auto option = parser.add_subcommand("option", "Operation on task's options");
  option->require_option(1);
  auto time_limit_option =
      option->add_option<int>("-l,--time-limit", time_limit, "Time limit in ms")->check(CLI::NonNegativeNumber);
  auto checker_option = option->add_option<string>("-c,--checker", checker_name, "Checker name");
  auto hide_option = option->add_flag("-o", "Toggle Hide accepted test cases option");
  auto truncate_flag = option->add_flag("-t,--truncate-long-test", "Toggle truncate long test option");
  auto interactive_flag = option->add_flag("-i,--interactive", "Toggle interactive option");
  auto stop_on_first_fail_flag = option->add_flag("-s,--stop-on-first-fail", "Toggle stop on first fail option");
  auto task_name_option = option->add_option<string>("-n,--name", task_name, "Task name");
  auto task_group_option = option->add_option<string>("-g,--group", task_group, "Task group");

  /**
   * operation on task's test generator
   */
  auto generator = parser.add_subcommand("gen", "Operation on task's generator");
  auto generator_know_ans_flag = generator->add_flag("-k,--known-gen-ans", "Toggle known gen ans option");
  auto generator_flag = generator->add_flag("-g,--generator", "Toggle use generator option");
  auto generator_num_test_option =
      generator->add_option<int>("-n,--num-test", generator_num_test, "Number of test to generate")
          ->check(CLI::NonNegativeNumber);
  auto generator_seed_option = generator->add_option<string>("-s,--seed", generator_seed, "Seed for generator");
  auto generator_params_option = generator->add_option<string>("-p,--params", generator_params, "Params for generator");

  test->excludes(option);
  test->excludes(generator);
  generator->excludes(option);

  parser.require_subcommand(1);

  try {
    parser.parse(argc, argv);
  } catch (const CLI::ParseError &e) {
    parser.exit(e);
    exit(1);
  }

  spdlog::debug("Config file is set to '{}'", config_path.generic_string());
  std::ifstream in_file(config_path.generic_string());
  json test_data = json::parse(in_file);
  spdlog::debug("Config file is parsed. Data is '{}'", test_data.dump(2));

  if (option->parsed()) {
    if (time_limit_option->count() > 0) {
      test_data["timeLimit"] = time_limit;
    }
    if (checker_option->count() > 0) {
      test_data["checker"] = checker_name;
    }
    if (truncate_flag->count() > 0) {
      test_data["truncateLongTest"] = !test_data["truncateLongTest"].get<bool>();
    }
    if (interactive_flag->count() > 0) {
      test_data["interactive"] = !test_data["interactive"].get<bool>();
    }
    if (hide_option->count() > 0) {
      test_data["hideAcceptedTestCases"] = !test_data["hideAcceptedTestCases"].get<bool>();
    }
    if (stop_on_first_fail_flag->count() > 0) {
      test_data["stopOnFirstFail"] = !test_data["stopOnFirstFail"].get<bool>();
    }
    if (task_name_option->count() > 0) {
      test_data["name"] = task_name;
    }
    if (task_group_option->count() > 0) {
      test_data["group"] = task_group;
    }
  }
  if (generator->parsed()) {
    spdlog::debug("Generator is parsed");
    if (generator_num_test_option->count() > 0) {
      test_data["numTest"] = generator_num_test;
    }
    if (generator_seed_option->count() > 0) {
      test_data["generatorSeed"] = generator_seed;
    }
    if (generator_params_option->count() > 0) {
      test_data["generatorParams"] = generator_params;
    }
    if (generator_know_ans_flag->count() > 0) {
      spdlog::debug("Generator know ans option is parsed");
      test_data["knowGenAns"] = !test_data["knowGenAns"].get<bool>();
    }
    if (generator_flag->count() > 0) {
      test_data["useGeneration"] = !test_data["useGeneration"].get<bool>();
    }
  }

  if (test_index != -1 && test_index >= test_data["tests"].size()) {
    cout << termcolor::red << termcolor::bold << "Test number " << test_index << " out of range" << termcolor::reset;
    cout << std::endl;
    exit(1);
  }

  if (print->parsed() || all->parsed() || none->parsed()) {
    if (test_index_vector.empty()) {
      spdlog::debug("Vector of tests to print is empty. Adding all tests.");
      for (int i = 0; i < test_data["tests"].size(); ++i) {
        test_index_vector.push_back(i);
      }
    }
    auto tmp = test_index_vector;
    for (int i = 0; i < (int)tmp.size(); i++) {
      if (tmp[i] < 0 || tmp[i] >= test_data["tests"].size()) {
        test_index_vector.erase(test_index_vector.begin() + i);
      }
    }
    if (test_index_vector.empty()) {
      cout << termcolor::yellow << termcolor::bold << "No tests to operate on" << termcolor::reset;
      cout << std::endl;
      exit(1);
    }
  }

  if (print->parsed()) {
    spdlog::debug("printing tests...");
    print_tests(test_index_vector, test_data);
    return 0;
  }

  if (all->parsed()) {
    for (auto i : test_index_vector) {
      test_data["tests"][i]["active"] = true;
    }
  }

  if (none->parsed()) {
    for (auto i : test_index_vector) {
      test_data["tests"][i]["active"] = false;
    }
  }

  // if (parser.count("print")) {
  //   spdlog::debug("Printing config file...");
  //   cout << test_data.dump(4) << std::endl;
  //   exit(0);
  // }
  if (unknow->parsed()) {
    spdlog::debug("Marking test {} as unknown", test_index);
    test_data["tests"][test_index]["answer"] = false;
  }

  if (add->parsed()) {
    spdlog::debug("adding test...");
    if (test_index == -1) {
      auto cache_dir = fs::temp_directory_path() / "cpcli_cli_task_editor";
      fs::create_directories(cache_dir);
      auto input_file = cache_dir / "input";
      auto output_file = cache_dir / "output";
      create_empty_file(input_file);
      create_empty_file(output_file);
      string command = "$EDITOR " + input_file.generic_string();
      if (!add_not_know_ans) {
        command += "  " + output_file.generic_string();
      }
      system_warper(command);
      test_data["tests"].push_back({
          {"active", !add_not_active},
          {"answer", !add_not_know_ans},
          {"input", rtrim_copy(read_file_to_str(input_file))},
          {"output", rtrim_copy(read_file_to_str(output_file))},
          {"index", test_data["tests"].size()},
      });
      fs::remove_all(cache_dir);
    } else {
      int num_test = test_data["tests"].size();
      spdlog::debug("copy test {} to {}", test_index, num_test);
      test_data["tests"].push_back({test_data["tests"][test_index]});
      test_data["tests"][num_test]["index"] = num_test;
      test_data["tests"][num_test]["active"] = !add_not_active;
      test_data["tests"][num_test]["answer"] = !add_not_know_ans;
    }
  }
  if (del->parsed()) {
    spdlog::debug("delete test #{}", test_index);
    test_data["tests"].erase(test_index);
    for (int i = test_index; i < test_data["tests"].size(); ++i) {
      test_data["tests"][i]["index"] = i;
    }
  }
  if (edit->parsed()) {
    spdlog::debug("editing test #{}", test_index);
    auto cache_dir = fs::temp_directory_path() / "cpcli_cli_task_editor";
    fs::create_directories(cache_dir);
    auto input_file = cache_dir / "input";
    auto output_file = cache_dir / "output";
    std::ofstream inf(input_file);
    std::ofstream ouf(output_file);
    inf << test_data["tests"][test_index]["input"].get<string>();
    ouf << test_data["tests"][test_index]["output"].get<string>();
    inf.close();
    ouf.close();
    string command = "$EDITOR " + input_file.generic_string() + "  " + output_file.generic_string();
    system_warper(command);
    test_data["tests"][test_index]["input"] = rtrim_copy(read_file_to_str(input_file));
    test_data["tests"][test_index]["output"] = rtrim_copy(read_file_to_str(output_file));
    fs::remove_all(cache_dir);
  }
  std::ofstream out_file(config_path.generic_string());
  out_file << test_data.dump(2);
  return 0;
}