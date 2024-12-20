cmake_minimum_required(VERSION 3.10)
project(test_rig_app CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-g)

# Add Dear ImGui with Docking
add_definitions(-DIMGUI_IMPL_OPENGL_LOADER_GLAD -DIMGUI_DEFINE_MATH_OPERATORS -DIMGUI_ENABLE_DOCKING -DIMGUI_ENABLE_VIEWPORTS)

# Include Dear ImGui
include_directories(
    imgui
    imgui/backends
)

# ImGui source files
set(IMGUI_SRC
    imgui/imgui.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_widgets.cpp
    imgui/imgui_tables.cpp
    imgui/backends/imgui_impl_glfw.cpp
    imgui/backends/imgui_impl_opengl3.cpp
)

# Include the attitude library
add_subdirectory(external/attitudeMathLibrary/)
include_directories(external/attitudeMathLibrary/include)

# Source files for the test_rig application
set(TEST_RIG_SOURCES
    src/main.cpp
    src/application.cpp
    src/renderer.cpp
    src/axis_renderer.cpp
    src/camera.cpp
)

add_executable(test_rig ${TEST_RIG_SOURCES} ${IMGUI_SRC})

# Link libraries
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(GLEW REQUIRED)

target_link_libraries(test_rig
    PRIVATE
    attitude
    OpenGL::GL
    glfw
    GLEW::GLEW
    dl             # Add this line to link the dynamic loading library
    stdc++         # Explicitly link the standard C++ library
)

# If attitude is set up as an imported or interface library,
# make sure to link properly and ensure that INTERFACE include dirs are set.
# Copy shaders to the build directory
file(COPY shaders/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/shaders)
