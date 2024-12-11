cmake_minimum_required(VERSION 3.10)
project(test_rig_app CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-g)

# Add the attitude library
add_subdirectory(external/attitudeMathLibrary/)

# Include directories for the attitude library if needed
# This might not be necessary if attitude sets INTERFACE include dirs
include_directories(external/attitudeMathLibrary/include)
include_directories(external/attitudeMathLibrary/include/attitude)

# Source files for the test_rig application
set(TEST_RIG_SOURCES
    src/main.cpp
    src/application.cpp
    src/renderer.cpp
    # ... other source files
)

add_executable(test_rig ${TEST_RIG_SOURCES})

# Link the attitude library and other dependencies (OpenGL, GLFW, etc.)
# For example, if you use GLFW for window creation:
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED) # or manually link a compiled GLFW
find_package(GLEW REQUIRED)  # optional if using modern OpenGL

target_link_libraries(test_rig
    PRIVATE
    attitude       # the library target from add_subdirectory
    OpenGL::GL
    glfw
    GLEW::GLEW
)


# If attitude is set up as an imported or interface library,
# make sure to link properly and ensure that INTERFACE include dirs are set.
# Copy shaders to the build directory
file(COPY shaders/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/shaders)
