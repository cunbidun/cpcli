#include "operations.hpp"
#include "config.hpp"
#include "util/color.hpp"
#include "util/util.hpp"

using std::cout;
using std::endl;

// TODO remove this
extern std::chrono::high_resolution_clock::time_point t_start;

void print_usage() {
  cout << "the number of parmameter is not correct!" << endl;
  cout << "usage: cpcli <path/to/folder> <path/to/project_config.json> [op_num]" << endl;
  cout << "op_num = 0 (default):  run normally" << endl;
  cout << "op_num = 1:            run with debug flags " << endl;
  cout << "op_num = 2:            run with terminal" << endl;
  cout << "op_num = 3:            test frontend" << endl;
  cout << "op_num = 4:            archive task" << endl;
  cout << endl;

  cout << "usage: cpcli <path/to/project_config.json> (for create new task)" << endl;
  exit(0);
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

  int status = system_wraper(join(command));

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

void edit_config(fs::path root_dir, fs::path &template_dir, fs::path &frontend_path) {

  namespace fs = fs;

  auto config = read_problem_config(root_dir / "config.json",
                                    template_dir / "config.template"); // reade the project config into a json object
  validate_problem_config(config);
  string old_name = config["name"].get<string>();

  // TODO pass this from env varibale
  string command = "java -jar \"" + frontend_path.string() + "\" \"" + root_dir.string() + "\"";
  system_wraper(command);

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
