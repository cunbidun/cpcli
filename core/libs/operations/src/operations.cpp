#include "operations.hpp"
#include "color.hpp"
#include "constant.hpp"
#include "path_manager.hpp"
#include "spdlog/spdlog.h"
#include "utils.hpp"

using std::cout;
using std::endl;

int create_new_task(nlohmann::json project_conf) {
  // dummy dir for new task
  std::string new_task = "___n3w_t4sk";

  PathManager path_manager;
  auto status = path_manager.init(project_conf);
  if (status != PathManagerStatus::Success) {
    spdlog::error("Path manager return non success code. Exiting...");
    exit(PathManagerFailToInitFromConfig);
  }
  TemplateManager template_manager(path_manager, "cpp", project_conf.value("use_template_engine", false));

  std::filesystem::path task_dir = path_manager.get_task();
  std::string frontend_exec = project_conf["frontend_exec"].get<std::string>();

  std::filesystem::create_directory(task_dir / new_task);
  std::filesystem::current_path(task_dir / new_task);
  std::filesystem::path root_dir = std::filesystem::current_path();

  template_manager.render(template_manager.get_solution(), root_dir / "solution.cpp", true);
  template_manager.render(template_manager.get_problem_config(), root_dir / "config.json", true);
  edit_config(task_dir / new_task, template_manager, frontend_exec);

  // read and validate the project config file
  nlohmann::json problem_conf = read_problem_config(root_dir / "config.json", template_manager.get_problem_config());

  std::string name = problem_conf["name"].get<std::string>();
  std::filesystem::current_path(root_dir.parent_path());
  if (name.size() == 0 || check_file(name, "")) {
    std::filesystem::remove_all(new_task);
  } else {
    std::filesystem::rename(new_task, name);
  }
  return 0;
}

int compile_cpp(std::filesystem::path &cache_dir,
                bool use_cache,
                const std::string &c_complier,
                std::filesystem::path &path,
                const std::string &compiler_flags,
                const std::string &binary_name) {
  std::filesystem::path binary_cache_dir = cache_dir / binary_name;
  std::filesystem::path file_cache_dir = cache_dir / path.filename();

  if (use_cache) {
    if (check_file(binary_cache_dir, "") && compare_files(path, file_cache_dir)) {
      std::filesystem::copy_file(binary_cache_dir,
                                 path.parent_path() / binary_name,
                                 std::filesystem::copy_options::overwrite_existing); // copy solution file to output dir
                                                                                     // for submission
      return 0;
    }
  }

  std::filesystem::current_path(path.parent_path());
  std::vector<std::string> command;

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
    std::filesystem::copy_file(
        path.parent_path() / binary_name, binary_cache_dir, std::filesystem::copy_options::overwrite_existing);
    std::filesystem::copy_file(path, file_cache_dir, std::filesystem::copy_options::overwrite_existing);
  }
  return status;
}

void print_report(const std::string report_name, bool passed, bool rte, bool tle, bool wa, long long runtime) {
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

void edit_config(std::filesystem::path root_dir, TemplateManager &template_manager, std::string &frontend_exec) {
  // read the project config into a nlohmann::json object
  nlohmann::json config = read_problem_config(root_dir / "config.json", template_manager.get_problem_config());
  std::string old_name = config["name"].get<std::string>();

  std::string command = frontend_exec + " \"" + root_dir.string() + "\"";
  system_warper(command);

  config = read_problem_config(root_dir / "config.json", template_manager.get_problem_config());
  std::string name = trim_copy(config["name"].get<std::string>());
  if (config["useGeneration"]) {
    template_manager.render(template_manager.get_gen(), root_dir / "gen.cpp", false);
  }
  if (config["interactive"]) {
    template_manager.render(template_manager.get_interactor(), root_dir / "interactor.cpp", false);
  }
  if (config["knowGenAns"]) {
    template_manager.render(template_manager.get_slow(), root_dir / "slow.cpp", false);
  }
  if (config["checker"].get<std::string>() == "custom") {
    template_manager.render(template_manager.get_checker(), root_dir / "checker.cpp", false);
  }
  if (name != old_name && name.size() != 0) {
    if (old_name.size() == 0) { // new task!!!
    } else {
      std::filesystem::current_path(root_dir.parent_path());
      std::filesystem::rename(old_name, name);
    }
  }
}

void print_duration(std::chrono::high_resolution_clock::time_point t_start) {
  auto t_end = std::chrono::high_resolution_clock::now();
  long long total_time = std::chrono::duration_cast<std::chrono::milliseconds>(t_end - t_start).count();
  cout << termcolor::magenta << termcolor::bold << "All testing finished in " << total_time << " ms" << termcolor::reset
       << endl;
}

bool check_file(std::filesystem::path path, const std::string &error_message) {
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
