# Slate Architecture

## Overview
Slate follows a **layered architecture** with clearly separated modules. No global mutable state, no singletons, no hidden dependencies. All modules communicate through well-defined public interfaces.

## Layer Diagram

| UI (Qt Widgets)          |
|--------------------------|
| CLI (Command Line)       |
|--------------------------|
| Scanner | Indexer | Graph|  (analysis)
|--------------------------|
| Storage (SQLite) | Core  |  (infrastructure)


## Module Responsibilities

### core
- Application lifecycle, logging factory.
- Provides ApplicationContext (no singleton).
- Dependency: spdlog.

### graph
- Generic directed graph data structure.
- Node/Edge types, cycle detection (DFS).
- Used by analysis modules and UI.

### storage
- SQLite database wrapper.
- Schema: files, symbols, dependencies, indexing_sessions.
- CRUD operations, migrations.
- Dependency: sqlite3, spdlog.

### scanner
- Recursive directory scanning.
- Language detection by extension.
- Symbol extraction (regex-based for MVP).
- Include/import parsing.
- Dependency: spdlog, std::filesystem.

### cli
- Command-line interface (slate scan, slate stats).
- Uses scanner, storage, and graph modules.

### ui (v0.2)
- Qt Widgets desktop application.
- Project tree view, metrics display.
- Integrates scanner and storage for live scanning.

## Design Principles
- **Composition over inheritance**
- **Public API documentation mandatory**
- **No global mutable state**
- **All dependencies explicit (injected)**
