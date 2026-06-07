#include "constant.hpp"
#include "operations.hpp"
#include "utils.hpp"
#include <cstdint>
#include <cstdlib>
#include <exception>
#include <fstream>
#include <toml.hpp>

using std::cout;

namespace {
std::string language_json_key(std::string language) {
  if (language.size() >= 2 && language.front() == '[' && language.back() == ']') {
    return language;
  }
  return "[" + language + "]";
}

nlohmann::json toml_value_to_json(const toml::value &value) {
  if (value.is_boolean()) {
    return value.as_boolean();
  }
  if (value.is_integer()) {
    return value.as_integer();
  }
  if (value.is_floating()) {
    return value.as_floating();
  }
  if (value.is_string()) {
    return value.as_string();
  }
  if (value.is_array()) {
    nlohmann::json array = nlohmann::json::array();
    for (const auto &item : value.as_array()) {
      array.push_back(toml_value_to_json(item));
    }
    return array;
  }
  if (value.is_table()) {
    nlohmann::json object = nlohmann::json::object();
    for (const auto &[key, item] : value.as_table()) {
      object[key] = toml_value_to_json(item);
    }
    return object;
  }
  return toml::format(value);
}

nlohmann::json normalize_language_config_keys(nlohmann::json j) {
  if (!j.contains("language_config") || !j["language_config"].is_object()) {
    return j;
  }

  nlohmann::json normalized = nlohmann::json::object();
  for (auto &[key, value] : j["language_config"].items()) {
    if (key == "default" || key == "override") {
      normalized[key] = value;
    } else {
      normalized[language_json_key(key)] = value;
    }
  }
  j["language_config"] = normalized;
  return j;
}

nlohmann::json parse_project_toml(const std::filesystem::path &path) {
  return normalize_language_config_keys(toml_value_to_json(toml::parse(path.string())));
}

toml::value json_to_toml_value(const nlohmann::json &value) {
  if (value.is_null()) {
    return toml::value();
  }
  if (value.is_boolean()) {
    return value.get<bool>();
  }
  if (value.is_number_integer()) {
    return value.get<std::int64_t>();
  }
  if (value.is_number_unsigned()) {
    return static_cast<std::int64_t>(value.get<std::uint64_t>());
  }
  if (value.is_number_float()) {
    return value.get<double>();
  }
  if (value.is_string()) {
    return value.get<std::string>();
  }
  if (value.is_array()) {
    toml::value::array_type array;
    for (const auto &item : value) {
      if (!item.is_null()) {
        array.push_back(json_to_toml_value(item));
      }
    }
    return array;
  }

  toml::value::table_type table;
  for (const auto &[key, item] : value.items()) {
    if (!item.is_null()) {
      table[key] = json_to_toml_value(item);
    }
  }
  return table;
}

nlohmann::json parse_problem_toml(const std::filesystem::path &path) {
  return toml_value_to_json(toml::parse(path.string()));
}

bool has_object(const nlohmann::json &j, const std::string &key) {
  return j.contains(key) && j[key].is_object();
}

void copy_if_present(nlohmann::json &dst, const nlohmann::json &src, const std::string &src_key, const std::string &dst_key) {
  if (src.contains(src_key) && !src[src_key].is_null()) {
    dst[dst_key] = src[src_key];
  }
}

nlohmann::json normalize_problem_config(nlohmann::json j) {
  nlohmann::json normalized = j;

  if (has_object(j, "problem")) {
    copy_if_present(normalized, j["problem"], "name", "name");
    copy_if_present(normalized, j["problem"], "group", "group");
    copy_if_present(normalized, j["problem"], "url", "url");
  }
  if (has_object(j, "run")) {
    copy_if_present(normalized, j["run"], "checker", "checker");
    copy_if_present(normalized, j["run"], "interactive", "interactive");
    copy_if_present(normalized, j["run"], "time_limit_ms", "timeLimit");
    copy_if_present(normalized, j["run"], "stop_on_first_failure", "stopAtWrongAnswer");
  }
  if (has_object(j, "display")) {
    copy_if_present(normalized, j["display"], "hide_accepted_tests", "hideAcceptedTest");
    copy_if_present(normalized, j["display"], "truncate_long_output", "truncateLongTest");
  }
  if (has_object(j, "generation")) {
    copy_if_present(normalized, j["generation"], "enabled", "useGeneration");
    copy_if_present(normalized, j["generation"], "test_count", "numTest");
    copy_if_present(normalized, j["generation"], "seed", "generatorSeed");
    copy_if_present(normalized, j["generation"], "args", "genParameters");
    copy_if_present(normalized, j["generation"], "expected_output_from_slow", "knowGenAns");
  }

  copy_if_present(normalized, j, "hideAcceptedTestCases", "hideAcceptedTest");
  copy_if_present(normalized, j, "stopOnFirstFail", "stopAtWrongAnswer");
  copy_if_present(normalized, j, "generatorParams", "genParameters");

  if (normalized.contains("tests") && normalized["tests"].is_array()) {
    for (size_t i = 0; i < normalized["tests"].size(); ++i) {
      auto &test = normalized["tests"][i];
      if (test.contains("enabled")) {
        test["active"] = test["enabled"];
      }
      if (test.contains("has_expected_output")) {
        test["answer"] = test["has_expected_output"];
      }
      if (!test.contains("index") || test["index"].is_null()) {
        test["index"] = i;
      }
      if (!test.contains("active")) {
        test["active"] = true;
      }
      if (!test.contains("answer")) {
        test["answer"] = test.contains("output") && !test["output"].is_null();
      }
    }
  }

  normalized.erase("problem");
  normalized.erase("run");
  normalized.erase("display");
  normalized.erase("generation");
  return normalized;
}

nlohmann::json problem_config_to_toml_schema(const nlohmann::json &config) {
  nlohmann::json out;
  out["problem"] = nlohmann::json::object();
  copy_if_present(out["problem"], config, "name", "name");
  copy_if_present(out["problem"], config, "group", "group");
  copy_if_present(out["problem"], config, "url", "url");

  out["run"] = nlohmann::json::object();
  copy_if_present(out["run"], config, "timeLimit", "time_limit_ms");
  copy_if_present(out["run"], config, "checker", "checker");
  copy_if_present(out["run"], config, "interactive", "interactive");
  copy_if_present(out["run"], config, "stopAtWrongAnswer", "stop_on_first_failure");

  out["display"] = nlohmann::json::object();
  copy_if_present(out["display"], config, "hideAcceptedTest", "hide_accepted_tests");
  copy_if_present(out["display"], config, "truncateLongTest", "truncate_long_output");

  out["generation"] = nlohmann::json::object();
  copy_if_present(out["generation"], config, "useGeneration", "enabled");
  copy_if_present(out["generation"], config, "numTest", "test_count");
  copy_if_present(out["generation"], config, "generatorSeed", "seed");
  copy_if_present(out["generation"], config, "genParameters", "args");
  copy_if_present(out["generation"], config, "knowGenAns", "expected_output_from_slow");

  out["tests"] = nlohmann::json::array();
  if (config.contains("tests") && config["tests"].is_array()) {
    for (const auto &test : config["tests"]) {
      nlohmann::json out_test;
      copy_if_present(out_test, test, "active", "enabled");
      copy_if_present(out_test, test, "answer", "has_expected_output");
      copy_if_present(out_test, test, "input", "input");
      copy_if_present(out_test, test, "output", "output");
      out["tests"].push_back(out_test);
    }
  }

  if (config.contains("languageConfig") && config["languageConfig"].is_object()) {
    out["language_config"] = config["languageConfig"];
  }
  return out;
}

std::filesystem::path expand_user_path(std::filesystem::path path) {
  auto str = path.string();
  if (str == "~" || str.rfind("~/", 0) == 0) {
    if (const char *home = std::getenv("HOME")) {
      return std::filesystem::path(home) / str.substr(str == "~" ? 1 : 2);
    }
  }
  return path;
}

std::string normalize_path_string(std::string value, const std::filesystem::path &base) {
  if (value.empty()) {
    return value;
  }
  std::filesystem::path path = expand_user_path(value);
  if (path.is_relative()) {
    path = base / path;
  }
  return path.lexically_normal().string();
}

void normalize_project_config(nlohmann::json &j, const std::filesystem::path &config_dir) {
  std::filesystem::path root = config_dir;
  if (j.contains("root") && !j["root"].is_null() && !j["root"].get<std::string>().empty()) {
    root = expand_user_path(j["root"].get<std::string>());
    if (root.is_relative()) {
      root = config_dir / root;
    }
  }
  j["root"] = root.lexically_normal().string();

  for (const auto &key : {"task", "archive", "output", "template"}) {
    if (j.contains(key) && j[key].is_string()) {
      j[key] = normalize_path_string(j[key].get<std::string>(), j["root"].get<std::string>());
    }
  }

  if (j.contains("language_config") && j["language_config"].is_object()) {
    for (auto &[_, language_config] : j["language_config"].items()) {
      if (language_config.is_object() && language_config.contains("include_dir") && language_config["include_dir"].is_string()) {
        language_config["include_dir"] = normalize_path_string(language_config["include_dir"].get<std::string>(), j["root"].get<std::string>());
      }
    }
  }

  if (j.contains("task_editor_exec") && j["task_editor_exec"].is_string()) {
    auto exec = j["task_editor_exec"].get<std::string>();
    if (exec == "~" || exec.rfind("~/", 0) == 0 || exec.find('/') != std::string::npos) {
      j["task_editor_exec"] = normalize_path_string(exec, j["root"].get<std::string>());
    }
  }
}
} // namespace

std::filesystem::path resolve_existing_project_config(std::filesystem::path path) {
  path = expand_user_path(path);
  if (path.is_relative()) {
    path = std::filesystem::current_path() / path;
  }
  if (std::filesystem::exists(path)) {
    return std::filesystem::canonical(path);
  }
  if (path.extension() == ".json") {
    auto toml_path = path;
    toml_path.replace_extension(".toml");
    if (std::filesystem::exists(toml_path)) {
      return std::filesystem::canonical(toml_path);
    }
  }
  return path;
}

bool validate_problem_config(const nlohmann::json &obj) { return true; }

nlohmann::json read_problem_config(std::filesystem::path path, std::filesystem::path template_path) {
  path = resolve_problem_config_path(path.parent_path());
  if (path.empty() || !check_file(path, "")) {
    cout << "Error, no config.toml or config.json file found. Please create one\n";
    cout << "Sample config:\n";
    print_file(template_path.string(), false);
    clean_up();
    exit(FILE_NOT_FOUND_ERR);
  }
  nlohmann::json j = parse_problem_config_file(path);
  if (!validate_problem_config(j)) {
    cout << "Invalid problem configuration\n";
    cout << "Sample config:\n";
    print_file(template_path.string(), false);
    clean_up();
    exit(INVALID_CONFIG_ERROR);
  }
  return j;
}

nlohmann::json parse_problem_config_file(std::filesystem::path path) {
  nlohmann::json j;
  if (path.extension() == ".toml" || path.extension() == ".template") {
    try {
      j = parse_problem_toml(path);
    } catch (const std::exception &e) {
      cout << "Invalid problem TOML configuration: " << e.what() << "\n";
      clean_up();
      exit(INVALID_CONFIG_ERROR);
    }
  } else {
    std::ifstream input(path);
    input >> j;
  }
  return normalize_problem_config(j);
}

std::filesystem::path resolve_problem_config_path(std::filesystem::path root_dir) {
  auto toml_path = root_dir / "config.toml";
  if (std::filesystem::exists(toml_path)) {
    return toml_path;
  }
  return root_dir / "config.json";
}

void write_problem_config(std::filesystem::path path, const nlohmann::json &config) {
  std::ofstream output(path);
  if (path.extension() == ".toml") {
    output << toml::format(json_to_toml_value(problem_config_to_toml_schema(normalize_problem_config(config))));
  } else {
    output << normalize_problem_config(config).dump(2);
  }
}

bool validate_project_config(const nlohmann::json &obj) { return true; }

nlohmann::json read_project_config(std::filesystem::path path) {
  path = resolve_existing_project_config(path);
  if (path.empty() || !check_file(path, "")) {
    cout << "Error, no project configuration file found. Please create one from template\n";
    clean_up();
    exit(FILE_NOT_FOUND_ERR);
  }
  std::ifstream input(path);
  nlohmann::json j;
  if (path.extension() == ".toml") {
    try {
      j = parse_project_toml(path);
    } catch (const std::exception &e) {
      cout << "Invalid project TOML configuration: " << e.what() << "\n";
      clean_up();
      exit(INVALID_CONFIG_ERROR);
    }
  } else {
    input >> j;
  }
  normalize_project_config(j, path.parent_path());
  if (!validate_project_config(j)) {
    cout << "Invalid project configuration\n";
    clean_up();
    exit(INVALID_CONFIG_ERROR);
  }
  return j;
}
