cmake_minimum_required(VERSION 3.15)

project(ace-nes VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_definitions(TOML_EXCEPTIONS=0)

set(SOURCE_FILES
    src/main.cpp
    src/cpu.hpp
    src/cpu.cpp
    src/decoder.hpp
    src/decoder.cpp
    src/instruction.hpp
    src/op.hpp
    src/types.hpp
)

set(ARGPARSE_BUILD_TESTS OFF)

include(cmake/raylib.cmake)
include(cmake/rlimgui.cmake)
include(cmake/imgui_club.cmake)

add_subdirectory(external/spdlog)
add_subdirectory(external/argparse)
add_subdirectory(external/magic_enum)
add_subdirectory(external/nativefiledialog-extended)
add_subdirectory(external/tomlplusplus)

add_executable(ace-nes ${SOURCE_FILES})

target_link_libraries(ace-nes spdlog)
target_link_libraries(ace-nes argparse)
target_link_libraries(ace-nes magic_enum)
target_link_libraries(ace-nes rlimgui)
target_link_libraries(ace-nes nfd)
target_link_libraries(ace-nes tomlplusplus::tomlplusplus)

target_include_directories(ace-nes PUBLIC external/rlimgui)

