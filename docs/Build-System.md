# Build System

## CMake Configuration
- **Minimum version**: CMake 3.21
- **C++ Standard**: C++23
- **Build targets**: Modern target-based CMake
- **No global include directories or compile flags**

## Dependencies
- **spdlog**: fetched via CMake FetchContent (v1.14.1)
- **sqlite3**: amalgamation placed in 	hird_party/sqlite3/
- **Qt 6**: required for UI (ind_package(Qt6 REQUIRED COMPONENTS Widgets))

## Build Targets
- slate_core (static library)
- slate_graph (static library)
- slate_storage (static library, links sqlite3 + spdlog)
- slate_scanner (static library, links spdlog)
- slate_cli (executable)
- slate_ui (executable, links Qt6::Widgets)

## Building
  ash
cmake -B build_qt -G "Ninja" -DCMAKE_BUILD_TYPE=Debug -DCMAKE_PREFIX_PATH="path/to/Qt"
cmake --build build_qt
  

## Code Quality
- clang-format for formatting
- clang-tidy for static analysis
- doctest for unit testing (to be integrated)
