cmake_minimum_required(VERSION 3.21)
project(Slate
    VERSION 0.1.0
    DESCRIPTION "Understand your codebase."
    LANGUAGES CXX C
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Sementara agar build tidak kosong
add_library(slate_core INTERFACE)
target_include_directories(slate_core INTERFACE core)

add_executable(slate_cli "")
target_link_libraries(slate_cli PRIVATE slate_core)
