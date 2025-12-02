# Tauri Task Editor

A modern replacement for the Java Swing Task Config Editor, built with Tauri 2 + Vite.

## Features

- 🎨 Follows system theme (dark/light mode)
- 🚀 Fast startup, low memory usage
- 📦 Small binary size (~10MB vs ~150MB for Electron)
- 🖥️ Cross-platform: Linux & macOS

## Development

### Prerequisites

Using Nix (recommended):

```bash
cd default/task_editor/tauri_task_editor
nix develop
```

Or manually install:
- Node.js 20+
- Rust (stable)
- Tauri dependencies (see [Tauri Prerequisites](https://tauri.app/start/prerequisites/))

### Install dependencies

```bash
npm install
```

### Run in development mode

```bash
npm run tauri dev -- -- --root /path/to/problem/directory
```

### Build for production

```bash
npm run tauri build
```

The binary will be in `src-tauri/target/release/task-editor`.

## Usage

```bash
task-editor --root /path/to/problem/directory
```

Or with short flag:

```bash
task-editor -r /path/to/problem/directory
```

## Project Structure

```
tauri_task_editor/
├── index.html          # Main HTML
├── src/
│   ├── main.js         # Frontend JavaScript
│   └── styles.css      # Styles (with dark mode support)
├── src-tauri/
│   ├── Cargo.toml      # Rust dependencies
│   ├── tauri.conf.json # Tauri configuration
│   └── src/
│       └── main.rs     # Rust backend
├── flake.nix           # Nix flake for building
├── package.json        # Node.js dependencies
└── vite.config.js      # Vite configuration
```
