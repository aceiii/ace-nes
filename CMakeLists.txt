cmake_minimum_required(VERSION 3.15)

project(ace-nes VERSION 0.0.1 LANGUAGES CXX)

if(APPLE)
    enable_language(OBJC)
    enable_language(OBJCXX)
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_definitions(TOML_EXCEPTIONS=0)

set(ARGPARSE_BUILD_TESTS OFF)

option(BUILD_NESTEST "Build nestest executable" OFF)
option(BUILD_SINGLE_STEP_TESTS "Build single step test executable" OFF)

include(cmake/imgui_club.cmake)
include(cmake/imgui.cmake)

add_subdirectory(external/spdlog)
add_subdirectory(external/argparse)
add_subdirectory(external/magic_enum)
add_subdirectory(external/nativefiledialog-extended)
add_subdirectory(external/tomlplusplus)
add_subdirectory(external/SDL)

add_library(ace-nes-shared STATIC
    src/address.hpp
    src/buffer.hpp
    src/bus.hpp
    src/cart.hpp
    src/cart.cpp
    src/cpu.hpp
    src/cpu.cpp
    src/decoder.hpp
    src/decoder.cpp
    src/emulator.hpp
    src/emulator.cpp
    src/file.hpp
    src/file.cpp
    src/instruction.hpp
    src/memory_map.hpp
    src/memory_map.cpp
    src/op.hpp
    src/ram.cpp
    src/ram.hpp
    src/string.hpp
    src/string.cpp
    src/types.hpp
)
target_link_libraries(ace-nes-shared spdlog)
target_link_libraries(ace-nes-shared argparse)
target_link_libraries(ace-nes-shared magic_enum)
target_link_libraries(ace-nes-shared nfd)
target_link_libraries(ace-nes-shared tomlplusplus::tomlplusplus)

add_executable(ace-nes
    src/main.cpp
    src/app.cpp
    src/app.hpp
    src/app_options.hpp
    src/args.hpp
    src/gui_app.cpp
    src/gui_app.hpp
    src/headless_app.cpp
    src/headless_app.hpp
    src/options_parser.cpp
    src/options_parser.cpp
)

target_link_libraries(ace-nes PRIVATE
    ace-nes-shared
    imgui::imgui
    SDL3::SDL3
)

if (BUILD_NESTEST)
    add_executable(nestest src/nestest.cpp)
    target_link_libraries(nestest PRIVATE ace-nes-shared)
endif()

if (BUILD_SINGLE_STEP_TESTS)
    add_subdirectory(external/simdjson)
    add_executable(single-step-tests src/single_step_tests.cpp)
    target_link_libraries(single-step-tests PRIVATE ace-nes-shared simdjson::simdjson)
endif()
