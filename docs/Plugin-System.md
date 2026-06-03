@'
# Plugin System (v0.4+)

## Overview
Slate will support a C ABI-based plugin system for language analysis, custom scanners, and UI extensions.

## Principles
- **Dynamic loading**: `.so` (Linux), `.dll` (Windows), `.dylib` (macOS)
- **Stable C ABI**: No raw C++ classes across boundaries
- **Version checking**: Plugins report compatible API version
- **Sandboxing**: (future) run plugins in separate process or Rust sandbox

## Plugin Interface (planned)
`  `c
typedef struct {
    int version;
    const char* name;
    void* (*init)(void);
    void  (*shutdown)(void* ctx);
    void  (*scan_file)(void* ctx, const char* path);
    // ...
} SlatePlugin;
` ` `
Loading Mechanism
- Plugin loader scans `plugins/` directory
- Validates version and loads shared library
- Registers with plugin registry (owned by `core` module)

Status
- Not implemented (planned for v0.4)
- Current architecture supports future integration via `plugin/` directory and `core` module