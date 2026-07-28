#![cfg_attr(not(debug_assertions), windows_subsystem = "windows")]

use serde::Deserialize;
use serde_json::{Map, Value};
use std::collections::BTreeSet;
use std::env;
use std::fs;
use std::path::PathBuf;
use std::process::Command;
use std::sync::Mutex;
use tauri::State;

struct AppState {
    config_read_path: Mutex<PathBuf>,
    config_write_path: Mutex<PathBuf>,
    project_config_path: Mutex<Option<PathBuf>>,
}

#[derive(Debug, Clone, Deserialize, Default)]
struct ProjectConfig {
    language_config: Option<Value>,
}

fn resolve_project_config_path(path: &PathBuf) -> PathBuf {
    if path.exists() {
        return path.clone();
    }
    if path.extension().and_then(|ext| ext.to_str()) == Some("json") {
        let mut toml_path = path.clone();
        toml_path.set_extension("toml");
        if toml_path.exists() {
            return toml_path;
        }
    }
    path.clone()
}

fn parse_project_config(path: &PathBuf, content: &str) -> Result<ProjectConfig, String> {
    match path.extension().and_then(|ext| ext.to_str()) {
        Some("toml") => toml::from_str(content)
            .map_err(|e| format!("Failed to parse project TOML config: {}", e)),
        _ => serde_json::from_str(content)
            .map_err(|e| format!("Failed to parse project JSON config: {}", e)),
    }
}

fn resolve_problem_config_read_path(root: &str) -> PathBuf {
    let root = PathBuf::from(root);
    let toml_path = root.join("config.toml");
    if toml_path.exists() {
        return toml_path;
    }
    root.join("config.json")
}

fn problem_config_write_path(read_path: &PathBuf) -> PathBuf {
    let mut write_path = read_path.clone();
    write_path.set_file_name("config.toml");
    write_path
}

fn copy_if_present(
    dst: &mut Map<String, Value>,
    src: &Map<String, Value>,
    src_key: &str,
    dst_key: &str,
) {
    if let Some(value) = src.get(src_key) {
        if !value.is_null() {
            dst.insert(dst_key.to_string(), value.clone());
        }
    }
}

fn normalize_problem_config_value(value: Value) -> Value {
    let Some(root) = value.as_object() else {
        return value;
    };

    let mut normalized = root.clone();
    if let Some(problem) = root.get("problem").and_then(|value| value.as_object()) {
        copy_if_present(&mut normalized, problem, "name", "name");
        copy_if_present(&mut normalized, problem, "group", "group");
        copy_if_present(&mut normalized, problem, "url", "url");
    }
    if let Some(run) = root.get("run").and_then(|value| value.as_object()) {
        copy_if_present(&mut normalized, run, "checker", "checker");
        copy_if_present(&mut normalized, run, "interactive", "interactive");
        copy_if_present(&mut normalized, run, "time_limit_ms", "timeLimit");
        copy_if_present(
            &mut normalized,
            run,
            "stop_on_first_failure",
            "stopAtWrongAnswer",
        );
    }
    if let Some(display) = root.get("display").and_then(|value| value.as_object()) {
        copy_if_present(
            &mut normalized,
            display,
            "hide_accepted_tests",
            "hideAcceptedTest",
        );
        copy_if_present(
            &mut normalized,
            display,
            "truncate_long_output",
            "truncateLongTest",
        );
    }
    if let Some(generation) = root.get("generation").and_then(|value| value.as_object()) {
        copy_if_present(&mut normalized, generation, "enabled", "useGeneration");
        copy_if_present(&mut normalized, generation, "test_count", "numTest");
        copy_if_present(&mut normalized, generation, "seed", "generatorSeed");
        copy_if_present(&mut normalized, generation, "args", "genParameters");
        copy_if_present(
            &mut normalized,
            generation,
            "expected_output_from_slow",
            "knowGenAns",
        );
    }

    copy_if_present(
        &mut normalized,
        root,
        "hideAcceptedTestCases",
        "hideAcceptedTest",
    );
    copy_if_present(
        &mut normalized,
        root,
        "stopOnFirstFail",
        "stopAtWrongAnswer",
    );
    copy_if_present(&mut normalized, root, "generatorParams", "genParameters");
    copy_if_present(&mut normalized, root, "language_config", "languageConfig");

    if let Some(tests) = normalized
        .get_mut("tests")
        .and_then(|value| value.as_array_mut())
    {
        for (index, test) in tests.iter_mut().enumerate() {
            let Some(test_obj) = test.as_object_mut() else {
                continue;
            };
            if let Some(enabled) = test_obj.get("enabled").cloned() {
                test_obj.insert("active".to_string(), enabled);
            }
            if let Some(has_expected_output) = test_obj.get("has_expected_output").cloned() {
                test_obj.insert("answer".to_string(), has_expected_output);
            }
            test_obj
                .entry("index".to_string())
                .or_insert(Value::from(index as i64));
            test_obj
                .entry("active".to_string())
                .or_insert(Value::Bool(true));
            let has_output = test_obj.get("output").is_some_and(|value| !value.is_null());
            test_obj
                .entry("answer".to_string())
                .or_insert(Value::Bool(has_output));
        }
    }

    Value::Object(normalized)
}

fn problem_config_to_toml_value(config: &Value) -> Result<Value, String> {
    let old = normalize_problem_config_value(config.clone());
    let root = old
        .as_object()
        .ok_or_else(|| "Problem config is not an object".to_string())?;

    let mut out = root.clone();
    let mut problem = root
        .get("problem")
        .and_then(Value::as_object)
        .cloned()
        .unwrap_or_default();
    copy_if_present(&mut problem, root, "name", "name");
    copy_if_present(&mut problem, root, "group", "group");
    copy_if_present(&mut problem, root, "url", "url");
    out.insert("problem".to_string(), Value::Object(problem));

    let mut run = root
        .get("run")
        .and_then(Value::as_object)
        .cloned()
        .unwrap_or_default();
    copy_if_present(&mut run, root, "timeLimit", "time_limit_ms");
    copy_if_present(&mut run, root, "checker", "checker");
    copy_if_present(&mut run, root, "interactive", "interactive");
    copy_if_present(&mut run, root, "stopAtWrongAnswer", "stop_on_first_failure");
    out.insert("run".to_string(), Value::Object(run));

    let mut display = root
        .get("display")
        .and_then(Value::as_object)
        .cloned()
        .unwrap_or_default();
    copy_if_present(
        &mut display,
        root,
        "hideAcceptedTest",
        "hide_accepted_tests",
    );
    copy_if_present(
        &mut display,
        root,
        "truncateLongTest",
        "truncate_long_output",
    );
    out.insert("display".to_string(), Value::Object(display));

    let mut generation = root
        .get("generation")
        .and_then(Value::as_object)
        .cloned()
        .unwrap_or_default();
    copy_if_present(&mut generation, root, "useGeneration", "enabled");
    copy_if_present(&mut generation, root, "numTest", "test_count");
    copy_if_present(&mut generation, root, "generatorSeed", "seed");
    copy_if_present(&mut generation, root, "genParameters", "args");
    copy_if_present(
        &mut generation,
        root,
        "knowGenAns",
        "expected_output_from_slow",
    );
    out.insert("generation".to_string(), Value::Object(generation));

    let mut tests = Vec::new();
    if let Some(old_tests) = root.get("tests").and_then(|value| value.as_array()) {
        for test in old_tests {
            let Some(test_obj) = test.as_object() else {
                continue;
            };
            let mut out_test = test_obj.clone();
            copy_if_present(&mut out_test, test_obj, "active", "enabled");
            copy_if_present(&mut out_test, test_obj, "answer", "has_expected_output");
            out_test.remove("active");
            out_test.remove("answer");
            out_test.remove("index");
            tests.push(Value::Object(out_test));
        }
    }
    out.insert("tests".to_string(), Value::Array(tests));

    if let Some(language_config) = root.get("languageConfig") {
        out.insert("language_config".to_string(), language_config.clone());
    }

    for key in [
        "name",
        "group",
        "url",
        "timeLimit",
        "checker",
        "interactive",
        "stopAtWrongAnswer",
        "hideAcceptedTest",
        "truncateLongTest",
        "useGeneration",
        "numTest",
        "generatorSeed",
        "genParameters",
        "knowGenAns",
        "languageConfig",
        "hideAcceptedTestCases",
        "stopOnFirstFail",
        "generatorParams",
    ] {
        out.remove(key);
    }

    Ok(Value::Object(out))
}

fn parse_problem_config(path: &PathBuf, content: &str) -> Result<Value, String> {
    let value = match path.extension().and_then(|ext| ext.to_str()) {
        Some("toml") => {
            let value: toml::Value = toml::from_str(content)
                .map_err(|e| format!("Failed to parse problem TOML config: {}", e))?;
            serde_json::to_value(value)
                .map_err(|e| format!("Failed to convert problem TOML config: {}", e))?
        }
        _ => serde_json::from_str(content)
            .map_err(|e| format!("Failed to parse problem JSON config: {}", e))?,
    };
    Ok(normalize_problem_config_value(value))
}

fn serialize_problem_config(config: &Value) -> Result<String, String> {
    let value = problem_config_to_toml_value(config)?;
    toml::to_string_pretty(&value)
        .map_err(|e| format!("Failed to serialize problem TOML config: {}", e))
}

#[tauri::command]
fn load_config(state: State<AppState>) -> Result<Value, String> {
    let config_path = state.config_read_path.lock().unwrap();
    let content = fs::read_to_string(&*config_path)
        .map_err(|e| format!("Failed to read config file: {}", e))?;
    let config = parse_problem_config(&config_path, &content)?;
    Ok(config)
}

#[tauri::command]
fn save_config(state: State<AppState>, config: Value) -> Result<(), String> {
    let config_path = state.config_write_path.lock().unwrap();
    let content = serialize_problem_config(&config)?;
    fs::write(&*config_path, content).map_err(|e| format!("Failed to write config file: {}", e))?;
    Ok(())
}

#[tauri::command]
fn load_language_options(state: State<AppState>) -> Result<Vec<String>, String> {
    let project_config_path = state.project_config_path.lock().unwrap();
    let Some(project_config_path) = project_config_path.as_ref() else {
        return Ok(Vec::new());
    };

    let project_config_path = resolve_project_config_path(project_config_path);
    let content = fs::read_to_string(&project_config_path)
        .map_err(|e| format!("Failed to read project config file: {}", e))?;
    let config = parse_project_config(&project_config_path, &content)?;

    let mut languages = BTreeSet::new();
    if let Some(Value::Object(language_config)) = config.language_config {
        for (key, value) in language_config {
            if key.starts_with('[') && key.ends_with(']') && key.len() > 2 {
                languages.insert(key[1..key.len() - 1].to_string());
            } else if value.is_object() && key != "override" {
                languages.insert(key.to_string());
            }
            if key == "default" {
                if let Some(language) = value.as_str() {
                    languages.insert(language.to_string());
                }
            }
            if key == "override" {
                if let Some(overrides) = value.as_object() {
                    for language in overrides.values().filter_map(|value| value.as_str()) {
                        languages.insert(language.to_string());
                    }
                }
            }
        }
    }

    Ok(languages.into_iter().collect())
}

#[tauri::command]
fn get_system_theme() -> String {
    // macOS: Check using defaults read
    #[cfg(target_os = "macos")]
    {
        if let Ok(output) = Command::new("defaults")
            .args(["read", "-g", "AppleInterfaceStyle"])
            .output()
        {
            let stdout = String::from_utf8_lossy(&output.stdout);
            if stdout.trim().to_lowercase() == "dark" {
                return "dark".to_string();
            }
        }
        // If the command fails or returns empty, it means light mode
        // (AppleInterfaceStyle key doesn't exist in light mode)
        return "light".to_string();
    }

    // Linux: Try gsettings for GTK color scheme
    #[cfg(target_os = "linux")]
    {
        if let Ok(output) = Command::new("gsettings")
            .args(["get", "org.gnome.desktop.interface", "color-scheme"])
            .output()
        {
            let stdout = String::from_utf8_lossy(&output.stdout);
            if stdout.contains("dark") {
                return "dark".to_string();
            } else if stdout.contains("light") {
                return "light".to_string();
            }
        }

        // Fallback: check GTK_THEME environment variable
        if let Ok(gtk_theme) = env::var("GTK_THEME") {
            if gtk_theme.to_lowercase().contains("dark") {
                return "dark".to_string();
            }
        }
        return "light".to_string();
    }

    // Default for other platforms
    #[cfg(not(any(target_os = "macos", target_os = "linux")))]
    "light".to_string()
}

fn main() {
    let args: Vec<String> = env::args().collect();

    // Parse command line arguments
    let mut root_path: Option<String> = None;
    let mut project_config_path: Option<PathBuf> = None;

    let mut i = 1;
    while i < args.len() {
        match args[i].as_str() {
            "-r" | "--root" => {
                if i + 1 < args.len() {
                    root_path = Some(args[i + 1].clone());
                    i += 2;
                } else {
                    eprintln!("Error: --root requires a value");
                    std::process::exit(1);
                }
            }
            "-p" | "--project-config" => {
                if i + 1 < args.len() {
                    project_config_path = Some(PathBuf::from(args[i + 1].clone()));
                    i += 2;
                } else {
                    eprintln!("Error: --project-config requires a value");
                    std::process::exit(1);
                }
            }
            "-h" | "--help" => {
                println!("Usage: task-editor [OPTIONS]");
                println!();
                println!("Options:");
                println!("  -r, --root <directory>  Path to the problem directory");
                println!("  -p, --project-config <file>  Path to the project config file");
                println!("  -h, --help              Show this help message");
                std::process::exit(0);
            }
            _ => {
                i += 1;
            }
        }
    }

    let config_read_path = match root_path {
        Some(root) => resolve_problem_config_read_path(&root),
        None => {
            eprintln!("Error: --root is required");
            std::process::exit(1);
        }
    };

    if !config_read_path.exists() {
        eprintln!("Error: Config file not found at {:?}", config_read_path);
        std::process::exit(1);
    }

    let config_write_path = problem_config_write_path(&config_read_path);

    tauri::Builder::default()
        .manage(AppState {
            config_read_path: Mutex::new(config_read_path),
            config_write_path: Mutex::new(config_write_path),
            project_config_path: Mutex::new(project_config_path),
        })
        .invoke_handler(tauri::generate_handler![
            load_config,
            save_config,
            get_system_theme,
            load_language_options
        ])
        .run(tauri::generate_context!())
        .expect("error while running tauri application");
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn editor_round_trip_preserves_unrecognized_config_data() {
        let input = r#"
custom_top_level = "keep me"

[problem]
name = "Python task"
group = "Tests"
custom_problem_key = 17

[run]
checker = "token_checker"
time_limit_ms = 3000
custom_run_key = "keep me too"

[language_config]
default = "py"
solution = "py"
custom_language_key = "still here"

[[tests]]
enabled = true
has_expected_output = true
input = "1\n"
output = "1\n"
custom_test_key = 42
"#;
        let path = PathBuf::from("config.toml");

        let config = parse_problem_config(&path, input).expect("config parses");
        let serialized = serialize_problem_config(&config).expect("config serializes");
        let value: toml::Value = toml::from_str(&serialized).expect("serialized TOML parses");

        assert_eq!(value["custom_top_level"].as_str(), Some("keep me"));
        assert_eq!(
            value["problem"]["custom_problem_key"].as_integer(),
            Some(17)
        );
        assert_eq!(value["run"]["custom_run_key"].as_str(), Some("keep me too"));
        assert_eq!(value["language_config"]["default"].as_str(), Some("py"));
        assert_eq!(
            value["language_config"]["custom_language_key"].as_str(),
            Some("still here")
        );
        assert_eq!(value["tests"][0]["custom_test_key"].as_integer(), Some(42));
    }
}
