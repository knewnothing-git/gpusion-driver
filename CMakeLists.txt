# CMakeLists.txt — GPUsion Driver (Linux CI Syntax Check)
#
# This CMake file is used ONLY for Linux CI syntax checking.
# It is NOT used for the real Windows driver build.
#
# Real Windows builds use the Windows Driver Kit (WDK) with MSBuild.
# See scripts/build.ps1 for the real build process.
#
# What this does on Linux:
#   - Compiles all driver C files with GCC
#   - Uses win_stubs.h to satisfy Windows API references
#   - Catches syntax errors and type mismatches in CI
#   - Does NOT produce a functional driver binary
#
# License: MIT
# Built in India 🇮🇳

cmake_minimum_required(VERSION 3.20)
project(gpusion_syntax_check C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Strict warnings — treat all warnings as errors in CI
add_compile_options(
    -Wall
    -Wextra
    -Wshadow
    -Wpointer-arith
    -Wcast-align
    -Wmissing-prototypes
    -Wstrict-prototypes
    -Werror
    # Suppress warnings that are unavoidable with Windows-style code
    -Wno-unused-parameter
    -Wno-sign-compare
)

# Tell our headers we are on Linux (enables win_stubs.h)
add_compile_definitions(
    __linux__
    _SYNTAX_CHECK_ONLY
)

# Include paths
include_directories(
    driver
    driver/compat
)

# All driver source files for syntax checking
set(DRIVER_SOURCES
    driver/kmdf/driver_entry.c
    driver/kmdf/backend_detect.c
    driver/wddm/add_device.c
    driver/wddm/start_device.c
    driver/wddm/query_adapter_info.c
    driver/wddm/submit_command.c
    driver/dxgi/dxgi_enum.c
    driver/vram/vram_proxy.c
)

# Build as a static library (not executable — we have no main())
add_library(gpusion_syntax_check STATIC ${DRIVER_SOURCES})

# Target properties
set_target_properties(gpusion_syntax_check PROPERTIES
    OUTPUT_NAME "gpusion_syntax_check"
)

# Print a reminder that this is NOT the real build
message(STATUS "")
message(STATUS "GPUsion CMake — Linux CI syntax check only")
message(STATUS "Real driver builds require Windows + WDK + MSBuild")
message(STATUS "See scripts/build.ps1 for the real build process")
message(STATUS "")
