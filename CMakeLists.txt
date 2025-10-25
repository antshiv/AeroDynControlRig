cmake_minimum_required(VERSION 3.10)
project(AeroDynControlRig CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-g)

# Add Dear ImGui with Docking
add_definitions(-DIMGUI_IMPL_OPENGL_LOADER_GLAD -DIMGUI_DEFINE_MATH_OPERATORS -DIMGUI_ENABLE_DOCKING -DIMGUI_ENABLE_VIEWPORTS)

# ImGui source files
set(IMGUI_SRC
    imgui/imgui.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_widgets.cpp
    imgui/imgui_tables.cpp
    imgui/backends/imgui_impl_glfw.cpp
    imgui/backends/imgui_impl_opengl3.cpp
)

# ImPlot source files (plotting library for ImGui)
set(IMPLOT_SRC
    external/implot/implot.cpp
    external/implot/implot_items.cpp
)

# Include the attitude library
add_subdirectory(external/attitudeMathLibrary/)

# Source files for the test_rig application
set(TEST_RIG_SOURCES
    src/app/main.cpp
    src/app/application.cpp
    src/modules/quaternion_demo.cpp
    src/modules/first_order_dynamics.cpp
    src/modules/sensor_simulator.cpp
    src/modules/complementary_estimator.cpp
    src/modules/rotor_telemetry.cpp
    src/gui/panel_manager.cpp
    src/gui/style.cpp
    src/gui/widgets/card.cpp
    src/gui/widgets/chip.cpp
    src/gui/panels/control_panel.cpp
    src/gui/panels/telemetry_panel.cpp
    src/gui/panels/dynamics_panel.cpp
    src/gui/panels/estimator_panel.cpp
    src/gui/panels/rotor_panel.cpp
    src/gui/panels/power_panel.cpp
    src/gui/panels/sensor_panel.cpp
    src/render/renderer.cpp
    src/render/axis_renderer.cpp
    src/render/camera.cpp
)

add_executable(AeroDynControlRig ${TEST_RIG_SOURCES} ${IMGUI_SRC} ${IMPLOT_SRC})

target_include_directories(AeroDynControlRig
    PRIVATE
        src
        imgui
        imgui/backends
        external/implot
        external/attitudeMathLibrary/include
)

# Link libraries
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(GLEW REQUIRED)

target_link_libraries(AeroDynControlRig
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
