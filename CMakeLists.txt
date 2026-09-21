cmake_minimum_required(VERSION 3.15)

project(ace-nes VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_definitions(TOML_EXCEPTIONS=0)

set(ARGPARSE_BUILD_TESTS OFF)

include(cmake/raylib.cmake)
include(cmake/rlimgui.cmake)
include(cmake/imgui_club.cmake)

add_subdirectory(external/spdlog)
add_subdirectory(external/argparse)
add_subdirectory(external/magic_enum)
add_subdirectory(external/nativefiledialog-extended)
add_subdirectory(external/tomlplusplus)

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
    src/file.hpp
    src/file.cpp
    src/instruction.hpp
    src/op.hpp
    src/types.hpp
)
target_link_libraries(ace-nes-shared spdlog)
target_link_libraries(ace-nes-shared argparse)
target_link_libraries(ace-nes-shared magic_enum)
target_link_libraries(ace-nes-shared rlimgui)
target_link_libraries(ace-nes-shared nfd)
target_link_libraries(ace-nes-shared tomlplusplus::tomlplusplus)
target_include_directories(ace-nes-shared PUBLIC external/rlimgui)

add_executable(ace-nes src/main.cpp)
target_link_libraries(ace-nes PRIVATE ace-nes-shared)

add_executable(nestest src/nestest.cpp)
target_link_libraries(nestest PRIVATE ace-nes-shared)
