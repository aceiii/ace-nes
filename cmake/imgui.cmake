
# set(imgui_SOURCES
#     "${CMAKE_SOURCE_DIR}/external/imgui/imgui.cpp"
#     "${CMAKE_SOURCE_DIR}/external/imgui/imgui_demo.cpp"
#     "${CMAKE_SOURCE_DIR}/external/imgui/imgui_draw.cpp"
#     "${CMAKE_SOURCE_DIR}/external/imgui/imgui_tables.cpp"
#     "${CMAKE_SOURCE_DIR}/external/imgui/imgui_widgets.cpp"
#     "${CMAKE_SOURCE_DIR}/external/imgui/misc/cpp/imgui_stdlib.cpp"
#     "${CMAKE_SOURCE_DIR}/external/imgui/backends/imgui_impl_sdl3.cpp"
#     "${CMAKE_SOURCE_DIR}/external/imgui/backends/imgui_impl_sdlrenderer3.cpp"
# )

add_library(imgui-static STATIC ${imgui_SOURCES})
add_library(imgui::imgui ALIAS imgui-static)

target_sources(imgui-static
    PRIVATE
        external/imgui/imgui.cpp
        external/imgui/imgui_demo.cpp
        external/imgui/imgui_draw.cpp
        external/imgui/imgui_tables.cpp
        external/imgui/imgui_widgets.cpp
        external/imgui/misc/cpp/imgui_stdlib.cpp
        external/imgui/backends/imgui_impl_sdl3.cpp
        external/imgui/backends/imgui_impl_sdlrenderer3.cpp
    PUBLIC
        external/imgui/imgui.h
        external/imgui/backends/imgui_impl_sdl3.h
        external/imgui/backends/imgui_impl_sdlrenderer3.h
)

target_include_directories(imgui-static PUBLIC external/imgui)

target_link_libraries(imgui-static PRIVATE SDL3::SDL3-static)
