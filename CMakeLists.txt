cmake_minimum_required(VERSION 3.15)

project(MyHex
    VERSION     1.0.0
    DESCRIPTION "Cross-platform TUI hex viewer"
    LANGUAGES   CXX
)

# ── C++ standard ──────────────────────────────────────────────────────────────
set(CMAKE_CXX_STANDARD          17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS        OFF)   # no -std=gnu++17; keep it portable

# ── Default build type ────────────────────────────────────────────────────────
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY
        STRINGS Debug Release RelWithDebInfo MinSizeRel)
endif()
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# ── Sources ───────────────────────────────────────────────────────────────────
add_executable(myhex
    src/main.cpp
    src/opts.cpp
    src/term.cpp
    src/render.cpp
    src/viewer.cpp
    src/dump.cpp
)

target_include_directories(myhex PRIVATE src)

# ── Warnings ──────────────────────────────────────────────────────────────────
target_compile_options(myhex PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /wd4996>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>
)

# ── Platform notes ────────────────────────────────────────────────────────────
# Windows : links kernel32 automatically (Console API).
#           _getch() is in msvcrt / libucrt (MinGW), no extra -l needed.
# POSIX   : termios and select() are in libc — no extra -l needed.
# No ncurses dependency.
#
# GCC 8.x / MinGW workaround:
#   std::ifstream inlined at -O1+ causes stack corruption on GCC 8.x + MinGW.
#   dump.cpp and viewer.cpp therefore use C-style FILE* instead of std::ifstream.

# ── Install ───────────────────────────────────────────────────────────────────
include(GNUInstallDirs)
install(TARGETS myhex RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
