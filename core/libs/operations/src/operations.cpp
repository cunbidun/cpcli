#include "operations.hpp"
#include "color.hpp"
#include "constant.hpp"
#include "path_manager.hpp"
#include "spdlog/spdlog.h"
#include "utils.hpp"

using std::cout;
using std::endl;

int create_new_task(json project_conf) {
  // dummy dir for new task
  string new_task = "___n3w_t4sk";

  PathManager path_manager;
  auto status = path_manager.init(project_conf);
  if (status != PathManagerStatus::Success) {
    spdlog::error("Path manager return non success code. Exiting...");
    exit(PathManagerFailToInitFromConfig);
  }
  TemplateManager template_manager(path_manager, "cpp");

  fs::path task_dir = path_manager.get_task();
  string frontend_exec = project_conf["frontend_exec"].get<string>();

  fs::create_directory(task_dir / new_task);
  fs::current_path(task_dir / new_task);
  fs::path root_dir = fs::current_path();

  fs::copy_file(template_manager.get_solution(), root_dir / "solution.cpp", fs::copy_options::overwrite_existing);
  fs::copy_file(template_manager.problem_config(), root_dir / "config.json", fs::copy_options::overwrite_existing);
  edit_config(task_dir / new_task, template_manager, frontend_exec);

  // read and validate the project config file
  json problem_conf = read_problem_config(root_dir / "config.json", template_manager.problem_config());

  string name = problem_conf["name"].get<string>();
  fs::current_path(root_dir.parent_path());
  if (name.size() == 0 || check_file(name, "")) {
    fs::remove_all(new_task);
  } else {
    fs::rename(new_task, name);
  }
  return 0;
}

int compile_cpp(fs::path &cache_dir,
                bool use_cache,
                const string &c_complier,
                fs::path &path,
                const string &compiler_flags,
                const string &binary_name) {
  fs::path binary_cache_dir = cache_dir / binary_name;
  fs::path file_cache_dir = cache_dir / path.filename();

  if (use_cache) {
    if (check_file(binary_cache_dir, "") && compare_files(path, file_cache_dir)) {
      copy_file(binary_cache_dir,
                path.parent_path() / binary_name,
                fs::copy_options::overwrite_existing); // copy solution file to output dir for
                                                       // submission
      return 0;
    }
  }

  fs::current_path(path.parent_path());
  std::vector<string> command;

  // build command
  command.push_back(c_complier);
  command.push_back(compiler_flags);
  command.push_back("-o");
  command.push_back(binary_name);
  command.push_back("\"" + path.string() + "\"");

  int status = system_warper(join(command));

  if (status != 0) {
    clean_up();
    exit(1); // TODO set correct return code
  }

  if (use_cache) {
    copy_file(path.parent_path() / binary_name, binary_cache_dir, fs::copy_options::overwrite_existing);
    copy_file(path, file_cache_dir, fs::copy_options::overwrite_existing);
  }
  return status;
}

void print_report(const string report_name, bool passed, bool rte, bool tle, bool wa, long long runtime) {
  using std::cout;
  cout << report_name << ": ";
  if (rte != 0) {
    cout << termcolor::red << termcolor::bold << "runtime error" << termcolor::reset << '\n';
  } else {
    auto run_time_color = termcolor::green;
    if (tle) {
      run_time_color = termcolor::red;
      cout << termcolor::red << termcolor::bold << "time limit exceeded" << termcolor::reset;
    } else if (passed) {
      cout << termcolor::green << termcolor::bold << "accepted" << termcolor::reset;
    } else if (wa) {
      cout << termcolor::red << termcolor::bold << "wrong answer" << termcolor::reset;
    } else {
      cout << termcolor::yellow << termcolor::bold << "undecided" << termcolor::reset;
    }
    cout << " in " << run_time_color << termcolor::bold << runtime << termcolor::reset << " ms\n";
  }
}

void edit_config(fs::path root_dir, TemplateManager &template_manager, string &frontend_exec) {
  // read the project config into a json object
  json config = read_problem_config(root_dir / "config.json", template_manager.problem_config());
  string old_name = config["name"].get<string>();

  string command = frontend_exec + " \"" + root_dir.string() + "\"";
  system_warper(command);

  config = read_problem_config(root_dir / "config.json", template_manager.problem_config());
  string name = trim_copy(config["name"].get<string>());
  // TODO check if template exists
  if (config["useGeneration"]) {
    copy_file(template_manager.get_gen(), root_dir / "gen.cpp", fs::copy_options::skip_existing);
  }
  if (config["interactive"]) {
    copy_file(template_manager.get_interactor(), root_dir / "interactor.cpp", fs::copy_options::skip_existing);
  }
  if (config["knowGenAns"]) {
    copy_file(template_manager.get_slow(), root_dir / "slow.cpp", fs::copy_options::skip_existing);
  }
  if (config["checker"].get<string>() == "custom") {
    copy_file(template_manager.get_checker(), root_dir / "checker.cpp", fs::copy_options::skip_existing);
  }
  if (name != old_name && name.size() != 0) {
    if (old_name.size() == 0) { // new task!!!
    } else {
      fs::current_path(root_dir.parent_path());
      fs::rename(old_name, name);
    }
  }
}

void print_duration(std::chrono::high_resolution_clock::time_point t_start) {
  auto t_end = std::chrono::high_resolution_clock::now();
  long long total_time = std::chrono::duration_cast<std::chrono::milliseconds>(t_end - t_start).count();
  cout << termcolor::magenta << termcolor::bold << "All testing finished in " << total_time << " ms" << termcolor::reset
       << endl;
}

bool check_file(std::filesystem::path path, const string &error_message) {
  if (std::filesystem::exists(path)) {
    return true;
  } else {
    if (error_message != "") {
      spdlog::error(error_message);
      clean_up();
      exit(FILE_NOT_FOUND_ERR);
    }
    return false;
  }
}
