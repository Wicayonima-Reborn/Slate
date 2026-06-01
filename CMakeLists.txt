cmake_minimum_required(VERSION 3.21)
project(Slate
    VERSION 0.1.0
    DESCRIPTION "Understand your codebase."
    LANGUAGES CXX C
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

# spdlog
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.14.1
)
FetchContent_MakeAvailable(spdlog)

# sqlite3 - manual dari third_party
add_library(sqlite3 STATIC third_party/sqlite3/sqlite3.c)
target_include_directories(sqlite3 PUBLIC third_party/sqlite3)

# --- slate_core ---
add_library(slate_core STATIC
    core/ApplicationContext.cpp
)
target_include_directories(slate_core PUBLIC core)
target_link_libraries(slate_core PUBLIC spdlog::spdlog)

# --- slate_graph ---
add_library(slate_graph STATIC
    graph/Graph.cpp
)
target_include_directories(slate_graph PUBLIC graph)

# --- slate_storage ---
add_library(slate_storage STATIC
    storage/Database.cpp
)
target_include_directories(slate_storage PUBLIC storage)
target_link_libraries(slate_storage PUBLIC sqlite3 spdlog::spdlog)

# --- slate_scanner ---
add_library(slate_scanner STATIC
    scanner/Scanner.cpp
)
target_include_directories(slate_scanner PUBLIC scanner)
target_link_libraries(slate_scanner PUBLIC spdlog::spdlog)

# --- slate_cli ---
add_executable(slate_cli
    cli/main.cpp
)
target_link_libraries(slate_cli PRIVATE slate_core slate_graph slate_storage slate_scanner)
