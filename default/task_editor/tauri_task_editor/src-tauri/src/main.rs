#![cfg_attr(not(debug_assertions), windows_subsystem = "windows")]

use serde::{Deserialize, Serialize};
use serde_json::{Map, Value};
use std::collections::BTreeSet;
use std::env;
use std::fs;
use std::path::PathBuf;
use std::process::Command;
use std::sync::Mutex;
use tauri::State;

#[derive(Debug, Clone, Serialize, Deserialize)]
#[serde(rename_all = "camelCase")]
struct Test {
    input: String,
    output: String,
    index: i32,
    active: bool,
    answer: bool,
}

#[derive(Debug, Clone, Serialize, Deserialize, Default)]
#[serde(rename_all = "camelCase")]
struct LanguageConfig {
    #[serde(skip_serializing_if = "Option::is_none")]
    solution: Option<String>,
    #[serde(skip_serializing_if = "Option::is_none")]
    slow: Option<String>,
    #[serde(skip_serializing_if = "Option::is_none")]
    gen: Option<String>,
    #[serde(skip_serializing_if = "Option::is_none")]
    checker: Option<String>,
    #[serde(skip_serializing_if = "Option::is_none")]
    interactor: Option<String>,
}

#[derive(Debug, Clone, Serialize, Deserialize, Default)]
#[serde(rename_all = "camelCase")]
struct ProblemConfig {
    #[serde(skip_serializing_if = "Option::is_none")]
    gen_parameters: Option<String>,
    #[serde(skip_serializing_if = "Option::is_none")]
    name: Option<String>,
    #[serde(skip_serializing_if = "Option::is_none")]
    group: Option<String>,
    #[serde(skip_serializing_if = "Option::is_none")]
    url: Option<String>,
    #[serde(skip_serializing_if = "Option::is_none")]
    checker: Option<String>,
    #[serde(skip_serializing_if = "Option::is_none")]
    num_test: Option<i64>,
    #[serde(skip_serializing_if = "Option::is_none")]
    time_limit: Option<i64>,
    #[serde(skip_serializing_if = "Option::is_none")]
    generator_seed: Option<String>,
    #[serde(skip_serializing_if = "Option::is_none")]
    use_generation: Option<bool>,
    #[serde(skip_serializing_if = "Option::is_none")]
    know_gen_ans: Option<bool>,
    #[serde(skip_serializing_if = "Option::is_none")]
    hide_accepted_test: Option<bool>,
    #[serde(skip_serializing_if = "Option::is_none")]
    truncate_long_test: Option<bool>,
    #[serde(skip_serializing_if = "Option::is_none")]
    stop_at_wrong_answer: Option<bool>,
    #[serde(skip_serializing_if = "Option::is_none")]
    interactive: Option<bool>,
    #[serde(default)]
    tests: Vec<Test>,
    #[serde(skip_serializing_if = "Option::is_none")]
    language_config: Option<LanguageConfig>,
}

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

fn copy_if_present(dst: &mut Map<String, Value>, src: &Map<String, Value>, src_key: &str, dst_key: &str) {
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
        copy_if_present(&mut normalized, run, "stop_on_first_failure", "stopAtWrongAnswer");
    }
    if let Some(display) = root.get("display").and_then(|value| value.as_object()) {
        copy_if_present(&mut normalized, display, "hide_accepted_tests", "hideAcceptedTest");
        copy_if_present(&mut normalized, display, "truncate_long_output", "truncateLongTest");
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

    copy_if_present(&mut normalized, root, "hideAcceptedTestCases", "hideAcceptedTest");
    copy_if_present(&mut normalized, root, "stopOnFirstFail", "stopAtWrongAnswer");
    copy_if_present(&mut normalized, root, "generatorParams", "genParameters");

    if let Some(tests) = normalized.get_mut("tests").and_then(|value| value.as_array_mut()) {
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
            test_obj.entry("index".to_string()).or_insert(Value::from(index as i64));
            test_obj.entry("active".to_string()).or_insert(Value::Bool(true));
            let has_output = test_obj.get("output").is_some_and(|value| !value.is_null());
            test_obj.entry("answer".to_string()).or_insert(Value::Bool(has_output));
        }
    }

    normalized.remove("problem");
    normalized.remove("run");
    normalized.remove("display");
    normalized.remove("generation");
    Value::Object(normalized)
}

fn problem_config_to_toml_value(config: &ProblemConfig) -> Result<Value, String> {
    let old = serde_json::to_value(config)
        .map_err(|e| format!("Failed to convert problem config: {}", e))?;
    let old = normalize_problem_config_value(old);
    let root = old
        .as_object()
        .ok_or_else(|| "Problem config is not an object".to_string())?;

    let mut out = Map::new();
    let mut problem = Map::new();
    copy_if_present(&mut problem, root, "name", "name");
    copy_if_present(&mut problem, root, "group", "group");
    copy_if_present(&mut problem, root, "url", "url");
    out.insert("problem".to_string(), Value::Object(problem));

    let mut run = Map::new();
    copy_if_present(&mut run, root, "timeLimit", "time_limit_ms");
    copy_if_present(&mut run, root, "checker", "checker");
    copy_if_present(&mut run, root, "interactive", "interactive");
    copy_if_present(&mut run, root, "stopAtWrongAnswer", "stop_on_first_failure");
    out.insert("run".to_string(), Value::Object(run));

    let mut display = Map::new();
    copy_if_present(&mut display, root, "hideAcceptedTest", "hide_accepted_tests");
    copy_if_present(&mut display, root, "truncateLongTest", "truncate_long_output");
    out.insert("display".to_string(), Value::Object(display));

    let mut generation = Map::new();
    copy_if_present(&mut generation, root, "useGeneration", "enabled");
    copy_if_present(&mut generation, root, "numTest", "test_count");
    copy_if_present(&mut generation, root, "generatorSeed", "seed");
    copy_if_present(&mut generation, root, "genParameters", "args");
    copy_if_present(&mut generation, root, "knowGenAns", "expected_output_from_slow");
    out.insert("generation".to_string(), Value::Object(generation));

    let mut tests = Vec::new();
    if let Some(old_tests) = root.get("tests").and_then(|value| value.as_array()) {
        for test in old_tests {
            let Some(test_obj) = test.as_object() else {
                continue;
            };
            let mut out_test = Map::new();
            copy_if_present(&mut out_test, test_obj, "active", "enabled");
            copy_if_present(&mut out_test, test_obj, "answer", "has_expected_output");
            copy_if_present(&mut out_test, test_obj, "input", "input");
            copy_if_present(&mut out_test, test_obj, "output", "output");
            tests.push(Value::Object(out_test));
        }
    }
    out.insert("tests".to_string(), Value::Array(tests));

    if let Some(language_config) = root.get("languageConfig") {
        out.insert("language_config".to_string(), language_config.clone());
    }

    Ok(Value::Object(out))
}

fn parse_problem_config(path: &PathBuf, content: &str) -> Result<ProblemConfig, String> {
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
    serde_json::from_value(normalize_problem_config_value(value))
        .map_err(|e| format!("Failed to normalize problem config: {}", e))
}

fn serialize_problem_config(config: &ProblemConfig) -> Result<String, String> {
    let value = problem_config_to_toml_value(config)?;
    toml::to_string_pretty(&value)
        .map_err(|e| format!("Failed to serialize problem TOML config: {}", e))
}

#[tauri::command]
fn load_config(state: State<AppState>) -> Result<ProblemConfig, String> {
    let config_path = state.config_read_path.lock().unwrap();
    let content = fs::read_to_string(&*config_path)
        .map_err(|e| format!("Failed to read config file: {}", e))?;
    let config = parse_problem_config(&config_path, &content)?;
    Ok(config)
}

#[tauri::command]
fn save_config(state: State<AppState>, config: ProblemConfig) -> Result<(), String> {
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
