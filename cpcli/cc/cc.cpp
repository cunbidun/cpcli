#include "crow.h"
#include "cxxopts.hpp"
#include "nlohmann/json.hpp"
#include "operations.hpp"
#include "path_manager.hpp"
#include "spdlog/spdlog.h"
#include "template_manager.hpp"

#include <filesystem>
#include <iostream>
#include <set>

const int CCNoConfigFileError = 10;
const int CCConfigFileNotFoundError = 11;
const int CCConfigFiletypeError = 11;

const int CCPort = 8080;

int main(int argc, char *argv[]) {
  using std::string;
  using json = nlohmann::json;
  namespace fs = std::filesystem;

  cxxopts::Options options("cpcli_cc", "Competitive Companion Server for cpcli_app");
  options.add_options()("p,project-config", "Project config", cxxopts::value<string>())("h,help", "Print usage");
  auto result = options.parse(argc, argv);

  if (result.count("help")) {
    std::cout << options.help() << std::endl;
    exit(0);
  }

  fs::path project_config_path;
  if (!result.count("project-config")) {
    std::cout << options.help() << std::endl;
    exit(CCNoConfigFileError);
  } else {
    project_config_path = result["project-config"].as<string>();
    if (!fs::exists(project_config_path)) {
      spdlog::error("Config file '{}' passed but not exists", project_config_path.generic_string());
      std::cout << options.help() << std::endl;
      exit(CCConfigFileNotFoundError);
    }
    if (fs::status(project_config_path).type() != fs::file_type::regular) {
      spdlog::error("Config file '{}' exists but not a regular file", project_config_path.generic_string());
      std::cout << options.help() << std::endl;
      exit(CCNoConfigFileError);
    }
  }
  spdlog::info("Config file is set to '{}'", project_config_path.generic_string());

  auto reformat = [](string s) -> string {
    replace(s.begin(), s.end(), '/', '-');
    auto remove_list = std::set({'[', ']', '"', '\'', '!', '#', ':'});
    s.erase(std::remove_if(s.begin(),
                           s.end(),
                           [&remove_list](auto const &c) -> bool { return remove_list.find(c) != remove_list.end(); }),
            s.end());
    return s;
  };

  auto project_conf = read_project_config(project_config_path); // read the project config into a json object
  PathManager path_manager;
  auto status = path_manager.init(project_conf);
  if (status != PathManagerStatus::Success) {
    spdlog::error("Path manager return non success code. Exiting...");
    exit(PathManagerFailToInitFromConfig);
  }
  TemplateManager template_manager(path_manager, "cpp");

  crow::SimpleApp cpcli;
  cpcli.loglevel(crow::LogLevel::WARNING);

  // define your endpoint at the root directory
  CROW_ROUTE(cpcli, "/").methods(crow::HTTPMethod::POST)([&](const crow::request &req) {
    auto data = json::parse(req.body);
    spdlog::info("Get request with body {} from server", data.dump(2));

    data["name"] = reformat(data["name"].get<string>());
    data["group"] = reformat(data["group"].get<string>());

    auto task_dir = path_manager.get_task() / data["name"].get<string>();

    // If the folder exists, abort creating task.
    if (fs::exists(task_dir)) {
      spdlog::info("Task '{}' already exists. Skip creating task.", task_dir.generic_string());
      return crow::response(crow::status::OK);
    }
    fs::create_directory(task_dir);

    auto solution_path = task_dir / "solution.cpp";
    copy_file(template_manager.get_solution(), solution_path);

    int cnt = 0;
    for (auto it = data["tests"].begin(); it != data["tests"].end(); ++it) {
      (*it)["index"] = cnt++;
      (*it)["active"] = true;
    }
    if (data.find("timeLimit") == data.end()) {
      data["timeLimit"] = 10000;
    }

    std::ifstream ifs(template_manager.get_config());
    auto final_config = json::parse(ifs);
    for (auto el : final_config.items()) {
      auto key = el.key();
      if (data.find(key) != data.end()) {
        final_config[key] = data[key];
      }
    }

    auto config_path = task_dir / "config.json";
    std::ofstream ofs(config_path);
    ofs << final_config.dump(2);

    return crow::response(crow::status::OK);
  });

  spdlog::info("Starting the Competitve Companion server at {}", CCPort);
  cpcli.port(CCPort).run();
}
