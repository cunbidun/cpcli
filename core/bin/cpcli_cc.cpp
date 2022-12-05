#include "CLI/CLI.hpp"
#include "constant.hpp"
#include "crow.h"
#include "nlohmann/json.hpp"
#include "operations.hpp"
#include "path_manager.hpp"
#include "spdlog/spdlog.h"
#include "template_manager.hpp"

#include <filesystem>
#include <iostream>
#include <set>

const int CCPort = 8080;

int main(int argc, char *argv[]) {
  using std::string;
  using json = nlohmann::json;
  namespace fs = std::filesystem;
  std::filesystem::path project_config_path;
  bool overwrite = false;
  CLI::App parser{"Competitive Companion Server for cpcli_app"};

  parser.add_option("-p,--project-config", project_config_path, "Path to the project config file")
      ->required(true)
      ->check(CLI::ExistingFile)
      ->transform([](std::filesystem::path path) { return std::filesystem::canonical(path); });

  parser.add_flag("-o,--overwrite", overwrite, "Overwrite existing task");

  parser.add_flag_function(
      "-d,--debug",
      [](int count) {
        spdlog::set_level(spdlog::level::debug);
        spdlog::debug("Debug (--debug) is set");
      },
      "Run with debug flags (this option will print debug logs)");

  try {
    parser.parse(argc, argv);
    if (overwrite) {
      spdlog::warn("cpcli_cc will overwrite existing tasks");
    }
  } catch (const CLI::ParseError &e) {
    parser.exit(e);
    exit(1);
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
  TemplateManager template_manager(path_manager, project_conf);

  crow::SimpleApp cpcli;
  cpcli.loglevel(crow::LogLevel::Warning);

  // define your endpoint at the root directory
  CROW_ROUTE(cpcli, "/").methods(crow::HTTPMethod::POST)([&](const crow::request &req) {
    auto data = json::parse(req.body);
    spdlog::info("Get request with body {} from server", data.dump(2));

    data["name"] = reformat(data["name"].get<string>());
    data["group"] = reformat(data["group"].get<string>());
    data["url"] = data["url"].get<string>();

    auto task_dir = path_manager.get_task() / data["name"].get<string>();

    // If the folder exists, abort creating task.
    if (std::filesystem::exists(task_dir)) {
      if (overwrite) {
        spdlog::warn("Task {} already exists. Overwriting...", data["name"].get<string>());
        std::filesystem::remove_all(task_dir);
      } else {
        spdlog::info("Task '{}' already exists. Skip creating task.", task_dir.generic_string());
        return crow::response(crow::status::OK);
      }
    }
    std::filesystem::create_directory(task_dir);

    template_manager.render(template_manager.get_solution(), task_dir, true);

    int cnt = 0;
    for (auto it = data["tests"].begin(); it != data["tests"].end(); ++it) {
      (*it)["index"] = cnt++;
      (*it)["active"] = true;
      (*it)["answer"] = true;
    }
    if (data.find("timeLimit") == data.end()) {
      data["timeLimit"] = 10000;
    }

    std::ifstream ifs(template_manager.get_problem_config());
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

  spdlog::info("Starting the Competitive Companion server at {}", CCPort);
  cpcli.port(CCPort).run();
}
