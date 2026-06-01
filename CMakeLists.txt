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

# sqlite3 amalgamation
FetchContent_Declare(
    sqlite3
    GIT_REPOSITORY https://github.com/azadkuh/sqlite-amalgamation.git
    GIT_TAG v3.43.1
)
FetchContent_MakeAvailable(sqlite3)
# Target sqlite3 tersedia dari FetchContent (sqlite3::sqlite3)

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
target_link_libraries(slate_storage PUBLIC sqlite3::sqlite3)

# --- slate_cli ---
add_executable(slate_cli
    cli/main.cpp
)
target_link_libraries(slate_cli PRIVATE slate_core slate_graph slate_storage)
