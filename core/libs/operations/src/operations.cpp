#include "operations.hpp"
#include "color.hpp"
#include "constant.hpp"
#include "path_manager.hpp"
#include "spdlog/spdlog.h"
#include "utils.hpp"
#include "inja.hpp"

using std::cout;
using std::endl;

int create_new_task(std::filesystem::path project_conf_path) {
  nlohmann::json project_conf = read_project_config(project_conf_path);
  // dummy dir for new task
  std::string new_task_name = "___n3w_t4sk";

  PathManager path_manager;
  auto status = path_manager.init(project_conf);
  if (status != PathManagerStatus::Success) {
    spdlog::error("Path manager return non success code. Exiting...");
    exit(PathManagerFailToInitFromConfig);
  }
  TemplateManager template_manager(path_manager, project_conf);

  std::filesystem::path task_dir = path_manager.get_task();
  std::string task_editor_exec = project_conf["task_editor_exec"].get<std::string>();

  std::filesystem::remove_all(new_task_name);
  std::filesystem::create_directory(task_dir / new_task_name);
  std::filesystem::current_path(task_dir / new_task_name);
  std::filesystem::path root_dir = std::filesystem::current_path();

  template_manager.render(template_manager.get_solution(), root_dir, true);
  template_manager.render(template_manager.get_problem_config(), root_dir, true);
  edit_config(task_dir / new_task_name, project_conf_path, template_manager, task_editor_exec);

  // read and validate the project config file
  nlohmann::json problem_conf = read_problem_config(root_dir / "config.json", template_manager.get_problem_config());

  std::string name = problem_conf["name"].get<std::string>();
  std::filesystem::current_path(root_dir.parent_path());
  if (name.size() == 0 || check_file(name, "")) {
    std::filesystem::remove_all(new_task_name);
  } else {
    std::filesystem::rename(new_task_name, name);
  }
  return 0;
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

void edit_config(std::filesystem::path root_dir,
                 std::filesystem::path project_conf_path,
                 TemplateManager &template_manager,
                 std::string &task_editor_exec) {
  // read the project config into a nlohmann::json object
  nlohmann::json config = read_problem_config(root_dir / "config.json", template_manager.get_problem_config());
  std::string old_name = config["name"].get<std::string>();

  std::string command_template = "{{ task_editor_exec }} --root '{{ root_dir }}' --project-config '{{ project_conf_path }}'";

  inja::Environment env;
  std::string command = env.render(command_template, {
    {"task_editor_exec", task_editor_exec},
    {"root_dir", root_dir.generic_string()},
    {"project_conf_path", project_conf_path .generic_string()}
  });

  system_warper(command);

  config = read_problem_config(root_dir / "config.json", template_manager.get_problem_config());
  std::string name = trim_copy(config["name"].get<std::string>());
  if (config["useGeneration"] && !template_manager.path_manager.check_task_path_exist(root_dir, "gen")) {
    template_manager.render(template_manager.get_gen(), root_dir, false);
  }
  if (config["interactive"] && !template_manager.path_manager.check_task_path_exist(root_dir, "interactor")) {
    template_manager.render(template_manager.get_interactor(), root_dir, false);
  }
  if (config["knowGenAns"] && !template_manager.path_manager.check_task_path_exist(root_dir, "slow")) {
    template_manager.render(template_manager.get_slow(), root_dir, false);
  }
  if (config["checker"].get<std::string>() == "custom" &&
      !template_manager.path_manager.check_task_path_exist(root_dir, "checker")) {
    template_manager.render(template_manager.get_checker(), root_dir, false);
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
