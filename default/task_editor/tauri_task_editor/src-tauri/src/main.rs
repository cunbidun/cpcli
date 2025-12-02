#![cfg_attr(not(debug_assertions), windows_subsystem = "windows")]

use serde::{Deserialize, Serialize};
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
    config_path: Mutex<PathBuf>,
}

#[tauri::command]
fn load_config(state: State<AppState>) -> Result<ProblemConfig, String> {
    let config_path = state.config_path.lock().unwrap();
    let content = fs::read_to_string(&*config_path)
        .map_err(|e| format!("Failed to read config file: {}", e))?;
    let config: ProblemConfig =
        serde_json::from_str(&content).map_err(|e| format!("Failed to parse config: {}", e))?;
    Ok(config)
}

#[tauri::command]
fn save_config(state: State<AppState>, config: ProblemConfig) -> Result<(), String> {
    let config_path = state.config_path.lock().unwrap();
    let content = serde_json::to_string_pretty(&config)
        .map_err(|e| format!("Failed to serialize config: {}", e))?;
    fs::write(&*config_path, content).map_err(|e| format!("Failed to write config file: {}", e))?;
    Ok(())
}

#[tauri::command]
fn get_system_theme() -> String {
    // Try to get GTK color scheme using gsettings
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
    
    // Default to light
    "light".to_string()
}

fn main() {
    let args: Vec<String> = env::args().collect();

    // Parse command line arguments
    let mut root_path: Option<String> = None;

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
            "-h" | "--help" => {
                println!("Usage: task-editor [OPTIONS]");
                println!();
                println!("Options:");
                println!("  -r, --root <directory>  Path to the problem directory");
                println!("  -h, --help              Show this help message");
                std::process::exit(0);
            }
            _ => {
                i += 1;
            }
        }
    }

    let config_path = match root_path {
        Some(root) => {
            let mut path = PathBuf::from(root);
            path.push("config.json");
            path
        }
        None => {
            eprintln!("Error: --root is required");
            std::process::exit(1);
        }
    };

    if !config_path.exists() {
        eprintln!("Error: Config file not found at {:?}", config_path);
        std::process::exit(1);
    }

    tauri::Builder::default()
        .manage(AppState {
            config_path: Mutex::new(config_path),
        })
        .invoke_handler(tauri::generate_handler![load_config, save_config, get_system_theme])
        .run(tauri::generate_context!())
        .expect("error while running tauri application");
}
