# Slate

**Understand your codebase.**

Slate is a cross-platform native codebase intelligence platform that helps developers understand, analyze, visualize, inspect, and monitor software projects.

Think of Slate as "Google Maps for software projects."

---

## Status

- **v0.1** ✅ Project scanning, dependency graph, SQLite storage, CLI (`slate scan`, `slate stats`)
- **v0.2** 🚧 Qt desktop application (project explorer, metrics viewer)
- **v0.3** 🔜 Semantic search, find references, graph viewer
- **v0.4** 🔜 Plugin system
- **v0.5** 🔜 Multi-language support (C, C++, Rust)
- **v1.0** 🔜 Architecture inspector

---

## Features

- Project scanning and indexing
- Symbol extraction (functions, classes, macros)
- Dependency graph with circular dependency detection
- SQLite local storage (offline-first)
- Command-line interface
- Qt desktop application (coming soon)

---

## Build

```bash
# Install dependencies: CMake, C++23 compiler, Qt 6
cmake -B build_qt -G "Ninja" -DCMAKE_BUILD_TYPE=Debug -DCMAKE_PREFIX_PATH="path/to/Qt"
cmake --build build_qt
```

---

## Usage
```bash
# Scan a project
./slate scan /path/to/project

# Show statistics
./slate stats slate.db
```

---

## License
MIT License. See [LICENSE](LICENSE) for details.

## Contributing
Pull requests welcome. See [docs](docs/) for architecture and coding standards.