# GLFW
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "")
set(GLFW_BUILD_TESTS OFF CACHE BOOL "")
set(GLFW_BUILD_DOCS OFF CACHE BOOL "")
set(GLFW_INSTALL OFF CACHE BOOL "")
add_subdirectory(glfw)
target_include_directories(glfw PUBLIC glfw/include)
if (APPLE)
   find_library(COCOA_LIBRARY Cocoa)
   find_library(IOKIT_LIBRARY IOKit)
   target_link_libraries(glfw INTERFACE ${COCOA_LIBRARY} ${IOKIT_LIBRARY})
endif ()
set_target_properties(glfw PROPERTIES FOLDER "ext/GLFW3")
set_target_properties(update_mappings PROPERTIES FOLDER "ext/GLFW3")

# Dear ImGui
add_library(imgui_glfw
    imgui/imgui.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
    imgui/imgui_demo.cpp
    imgui/misc/cpp/imgui_stdlib.cpp
    imgui/backends/imgui_impl_glfw.cpp
    imgui/backends/imgui_impl_opengl3.cpp
)
target_include_directories(imgui_glfw PUBLIC imgui imgui/backends)
target_link_libraries(imgui_glfw glfw ${OPENGL_LIBRARY})
set_target_properties(imgui_glfw PROPERTIES FOLDER "ext")

add_library(imgui_glfw_opengl2
    imgui/imgui.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
    imgui/backends/imgui_impl_glfw.cpp
    imgui/backends/imgui_impl_opengl2.cpp
)
target_include_directories(imgui_glfw_opengl2 PUBLIC imgui imgui/backends)
target_link_libraries(imgui_glfw_opengl2 glfw ${OPENGL_LIBRARY})
set_target_properties(imgui_glfw_opengl2 PROPERTIES FOLDER "ext")

add_executable(imgui_demo
    imgui/imgui_demo.cpp
    imgui/examples/example_glfw_opengl3/main.cpp)
target_link_libraries(imgui_demo imgui_glfw)
set_target_properties(imgui_demo PROPERTIES FOLDER "ext")

# https://github.com/thedmd/imgui-node-editor
add_library(imgui-node-editor
    imgui-node-editor/crude_json.cpp
    imgui-node-editor/imgui_canvas.h
    imgui-node-editor/imgui_node_editor_api.cpp
    imgui-node-editor/crude_json.h
    imgui-node-editor/imgui_extra_math.h
    imgui-node-editor/imgui_node_editor_internal.h
    imgui-node-editor/imgui_bezier_math.h
    imgui-node-editor/imgui_extra_math.inl
    imgui-node-editor/imgui_node_editor_internal.inl
    imgui-node-editor/imgui_bezier_math.inl
    imgui-node-editor/imgui_node_editor.cpp
    imgui-node-editor/imgui_canvas.cpp
    imgui-node-editor/imgui_node_editor.h
)
target_include_directories(imgui-node-editor PUBLIC imgui-node-editor)
target_link_libraries(imgui-node-editor imgui_glfw)
if (MSVC)
    target_compile_options(imgui-node-editor PRIVATE /wd4390)
endif ()
set_target_properties(imgui-node-editor PROPERTIES FOLDER "ext")
