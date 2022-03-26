#include "operations.hpp"
#include "config.hpp"
#include "constant.hpp"
#include "util/color.hpp"
#include "util/util.hpp"

using std::cout;
using std::endl;

extern std::chrono::high_resolution_clock::time_point t_start; // TODO remove this

// TODO fix this
void print_usage() {
  cout << endl;
  cout << "usage: cpcli -p path/to/project_config.json [OPTION]" << endl;
  cout << endl;

  cout << "Required: all options require this to be set" << endl;
  cout << "   -p, --project-config          path to the golbal config file" << endl;
  cout << endl;

  cout << "Optional: except for creating new task, all options require this to be set" << endl;
  cout << "   -r, --root-dir                the folder directory" << endl;
  cout << endl;

  cout << "Optional flags: those arguments do not require an arguments" << endl;
  cout << "   -a, --archive                 archive current root-dir" << endl;
  cout << "   -b, --build                   build solution file with normal flags" << endl;
  cout << "   -B, --build-with-term         build solution file with terminal (this option will not use cpcli)" << endl;
  cout << "   -d, --build-with-debug        build solution file with debug flags" << endl;
  cout << "   -n, --new                     create new task (this option does not require --root-dir)" << endl;
  cout << endl;

  cout << "Info: those arguments do not require an arguments" << endl;
  cout << "   -D, --debug                   run cpcli_app with debug flags (this option will print debug logs)" << endl;
  cout << "                                 when running with this flag, it is best to put this before others" << endl;
  cout << "   -h, --help                    show this help" << endl;
  cout << "   -v, --version                 print cpcli_app version" << endl;
  cout << endl;

  cout << "Examples:" << endl;
  cout << "   cpcli_app --new --project-config=./project_config.json" << endl;
  cout << "       for creating new task (the location is determine in the config file)" << endl;
  cout << endl;

  cout << "   cpcli_app --root-dir='$ROOT' --project-config=./project_config.json --build-with-debug" << endl;
  cout << "       for building and running task at '$ROOT' with debug flags" << endl;
  cout << endl;

  cout << "   cpcli_app -D --root-dir='$ROOT' --project-config=./project_config.json --build-with-debug" << endl;
  cout << "       for building and running task at '$ROOT' with debug flags, also show logs from cpcli_app" << endl;
  cout << endl;

  cout << "Check out https://github.com/cunbidun/cpcli for more info";
  cout << endl;
}

int create_new_task(json project_conf) {
  // dummy dir for new task
  string new_task = "___n3w_t4sk";

  fs::path task_dir = fs::absolute(project_conf["task_dir"].get<string>());
  string frontend_exec = project_conf["frontend_exec"].get<string>();
  fs::path template_dir = fs::absolute(project_conf["template_dir"].get<string>());

  fs::create_directory(task_dir / new_task);
  fs::current_path(task_dir / new_task);
  fs::path root_dir = fs::current_path();

  fs::copy_file(template_dir / "solution.template", root_dir / "solution.cpp", fs::copy_options::overwrite_existing);
  fs::copy_file(template_dir / "config.template", root_dir / "config.json", fs::copy_options::overwrite_existing);
  edit_config(task_dir / new_task, template_dir, frontend_exec);

  // read and validate the project config file
  json problem_conf = read_problem_config(root_dir / "config.json", template_dir / "config.template");
  validate_problem_config(problem_conf);

  string name = problem_conf["name"].get<string>();
  fs::current_path(root_dir.parent_path());
  if (name.size() == 0 || check_dir(name, "")) {
    fs::remove_all(new_task);
  } else {
    fs::rename(new_task, name);
  }
  return 0;
}

int clean_up(int first_time) {
  // Remove binary and ___test_case directory everytime
  fs::remove("solution");
  fs::remove("checker");
  fs::remove("gen");
  fs::remove("slow");
  fs::remove("interactor");
  fs::remove("printer");
  fs::remove_all("___test_case");

  // if not the first_time, assume that testing is done
  if (!first_time) {
    auto t_end = std::chrono::high_resolution_clock::now();
    long long total_time = std::chrono::duration_cast<std::chrono::milliseconds>(t_end - t_start).count();
    cout << termcolor::magenta << termcolor::bold << "All testing finished in " << total_time << " ms"
         << termcolor::reset << endl;
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
    copy_file(path.parent_path() / binary_name,
              binary_cache_dir,
              fs::copy_options::overwrite_existing); // copy solution file to output dir for
                                                     // submission
    copy_file(path,
              file_cache_dir,
              fs::copy_options::overwrite_existing); // copy solution file to output dir for
                                                     // submission
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

void sigint() {
  cout << endl;
  cout << termcolor::red << termcolor::bold << "SIGINT encoutered\n";
  clean_up();
  exit(0);
}

void edit_config(fs::path root_dir, fs::path &template_dir, string &frontend_exec) {

  namespace fs = fs;

  auto config = read_problem_config(root_dir / "config.json",
                                    template_dir / "config.template"); // reade the project config into a json object
  validate_problem_config(config);
  string old_name = config["name"].get<string>();

  // TODO pass this from env varibale
  string command = frontend_exec + " \"" + root_dir.string() + "\"";
  system_warper(command);

  config = read_problem_config(root_dir / "config.json", template_dir / "config.template");
  string name = trim_copy(config["name"].get<string>());
  // TODO check if template exists
  if (config["useGeneration"]) {
    copy_file(template_dir / "gen.template", root_dir / "gen.cpp", fs::copy_options::skip_existing);
  }
  if (config["interactive"]) {
    copy_file(template_dir / "interactor.template", root_dir / "interactor.cpp", fs::copy_options::skip_existing);
  }
  if (config["knowGenAns"]) {
    copy_file(template_dir / "slow.template", root_dir / "slow.cpp", fs::copy_options::skip_existing);
  }
  if (config["checker"].get<string>() == "custom") {
    copy_file(template_dir / "checker.template", root_dir / "checker.cpp", fs::copy_options::skip_existing);
  }
  if (name != old_name && name.size() != 0) {
    if (old_name.size() == 0) { // new task!!!
    } else {
      fs::current_path(root_dir.parent_path());
      fs::rename(old_name, name);
    }
  }
}
