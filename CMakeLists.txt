cmake_minimum_required(VERSION 3.16)
project(mini_readelf VERSION 1.0.0 LANGUAGES CXX)

# ── C++17 required for structured bindings and std::optional ─────────────────
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ── Source files ──────────────────────────────────────────────────────────────
set(SOURCES
    src/elf_parser.cpp
    src/main.cpp
)

set(HEADERS
    src/elf_types.h
    src/elf_parser.h
)

add_executable(elf-parser ${SOURCES} ${HEADERS})

target_include_directories(elf-parser PRIVATE src)

# ── Compiler warnings ─────────────────────────────────────────────────────────
if(MSVC)
    target_compile_options(elf-parser PRIVATE
        /W4
        /wd4996   # 'function' declared deprecated  (POSIX names)
    )
else()
    target_compile_options(elf-parser PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -Wshadow
        -Wno-unused-function
    )
endif()

# ── Release build optimisations ───────────────────────────────────────────────
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# ── Install ───────────────────────────────────────────────────────────────────
install(TARGETS elf-parser DESTINATION bin)

message(STATUS "Build type  : ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ standard: C++${CMAKE_CXX_STANDARD}")
message(STATUS "Compiler    : ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
