cmake_minimum_required(VERSION 3.10)
project(AeroDynControlRig LANGUAGES C CXX)

include(CTest)
find_package(PkgConfig REQUIRED)
pkg_check_modules(JSONC REQUIRED IMPORTED_TARGET json-c)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-g)
option(BUILD_AERODYN_GUI "Build the OpenGL/ImGui desktop application" ON)

add_custom_target(aerodyn_generate_aircraft_geometry
    COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/tools/generate_aircraft_geometry.sh"
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    COMMENT "Generating FreeCAD, STEP, STL, and OBJ aircraft artifacts")

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

# Include the dynamic models library
add_subdirectory(external/dynamic_models/)
set(CS_BUILD_TESTS OFF CACHE BOOL "Disable nested controlSystems tests" FORCE)
add_subdirectory(external/controlSystems/)
add_subdirectory(external/asr_fc/)

if(BUILD_AERODYN_GUI)
  # Source files for the test_rig application
  set(TEST_RIG_SOURCES
    src/app/main.cpp
    src/app/application.cpp
    src/modules/quaternion_demo.cpp
    src/modules/quadcopter_dynamics.cpp
    src/modules/flight_control.cpp
    src/modules/mission_control.cpp
    src/modules/nrf_hil.cpp
    src/hil/nrf_hil_transport.cpp
    src/modules/first_order_dynamics.cpp
    src/modules/sensor_simulator.cpp
    src/modules/complementary_estimator.cpp
    src/modules/rotor_telemetry.cpp
    src/config/aircraft_config.cpp
    src/input/joystick_input.cpp
    src/gui/panel_manager.cpp
    src/gui/style.cpp
    src/gui/widgets/card.cpp
    src/gui/widgets/chip.cpp
    src/gui/panels/control_panel.cpp
    src/gui/panels/joystick_panel.cpp
    src/gui/panels/mathematical_model_panel.cpp
    src/gui/panels/telemetry_panel.cpp
    src/gui/panels/dynamics_panel.cpp
    src/gui/panels/estimator_panel.cpp
    src/gui/panels/rotor_panel.cpp
    src/gui/panels/power_panel.cpp
    src/gui/panels/sensor_panel.cpp
    src/gui/panels/rotor_analysis_panel.cpp
    src/render/renderer.cpp
    src/render/obj_mesh.cpp
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
        external/dynamic_models/include
        external/dynamic_models/external/attitudeMathLibrary/include
  )

# Link libraries
  find_package(OpenGL REQUIRED)
  find_package(glfw3 REQUIRED)
  find_package(GLEW REQUIRED)

  target_link_libraries(AeroDynControlRig
    PRIVATE
    dynamic_models
    control_systems
    asr_fc_motor_link
    OpenGL::GL
    glfw
    GLEW::GLEW
    dl             # Add this line to link the dynamic loading library
    stdc++         # Explicitly link the standard C++ library
    PkgConfig::JSONC
  )

  target_compile_definitions(AeroDynControlRig PRIVATE
    ASR_DEFAULT_AIRCRAFT_SPEC="${CMAKE_CURRENT_SOURCE_DIR}/config/reference-quad.aircraft.json"
    ASR_DEFAULT_COEFFICIENT_BUNDLE="${CMAKE_CURRENT_SOURCE_DIR}/config/reference-quad.coefficients.json"
    ASR_DEFAULT_AIRCRAFT_MESH="${CMAKE_CURRENT_SOURCE_DIR}/assets/generated/asr-reference-quad.obj")
endif()

if(BUILD_TESTING AND UNIX)
    find_package(Threads REQUIRED)
    add_executable(aerodyn_nrf_hil_probe
        tools/nrf_hil_probe.cpp
        src/hil/nrf_hil_transport.cpp)
    target_include_directories(aerodyn_nrf_hil_probe
        PRIVATE src external/asr_fc/include)
    target_link_libraries(aerodyn_nrf_hil_probe PRIVATE asr_fc_motor_link)

    add_executable(aerodyn_nrf_hil_transport_test
        tests/test_nrf_hil_transport.cpp
        src/hil/nrf_hil_transport.cpp)
    target_include_directories(aerodyn_nrf_hil_transport_test
        PRIVATE src external/asr_fc/include)
    target_link_libraries(aerodyn_nrf_hil_transport_test
        PRIVATE asr_fc_motor_link Threads::Threads util)
    add_test(NAME aerodyn_nrf_hil_transport_test
        COMMAND aerodyn_nrf_hil_transport_test)
endif()

if(BUILD_TESTING)
    find_path(GLM_INCLUDE_DIR glm/glm.hpp)
    if(GLM_INCLUDE_DIR)
        add_executable(aerodyn_headless_plant_test
            tests/test_quadcopter_dynamics.cpp
            src/modules/quadcopter_dynamics.cpp
            src/config/aircraft_config.cpp
        )
        target_include_directories(aerodyn_headless_plant_test
            PRIVATE
                src
                ${GLM_INCLUDE_DIR}
                external/dynamic_models/include
                external/dynamic_models/external/attitudeMathLibrary/include
        )
        target_link_libraries(aerodyn_headless_plant_test PRIVATE dynamic_models PkgConfig::JSONC)
        target_compile_definitions(aerodyn_headless_plant_test PRIVATE
            ASR_DEFAULT_AIRCRAFT_SPEC="${CMAKE_CURRENT_SOURCE_DIR}/config/reference-quad.aircraft.json"
            ASR_DEFAULT_COEFFICIENT_BUNDLE="${CMAKE_CURRENT_SOURCE_DIR}/config/reference-quad.coefficients.json")
        add_test(NAME aerodyn_headless_plant_test COMMAND aerodyn_headless_plant_test)

        add_executable(aerodyn_flight_control_test
            tests/test_flight_control.cpp
            src/modules/flight_control.cpp
            src/modules/quadcopter_dynamics.cpp
            src/config/aircraft_config.cpp)
        target_include_directories(aerodyn_flight_control_test
            PRIVATE src external/controlSystems/include ${GLM_INCLUDE_DIR})
        target_link_libraries(aerodyn_flight_control_test
            PRIVATE control_systems dynamic_models PkgConfig::JSONC)
        target_compile_definitions(aerodyn_flight_control_test PRIVATE
            ASR_DEFAULT_AIRCRAFT_SPEC="${CMAKE_CURRENT_SOURCE_DIR}/config/reference-quad.aircraft.json"
            ASR_DEFAULT_COEFFICIENT_BUNDLE="${CMAKE_CURRENT_SOURCE_DIR}/config/reference-quad.coefficients.json")
        add_test(NAME aerodyn_flight_control_test COMMAND aerodyn_flight_control_test)

        add_executable(aerodyn_mode2_response_test
            tests/test_mode2_response.cpp
            src/modules/flight_control.cpp
            src/modules/quadcopter_dynamics.cpp
            src/config/aircraft_config.cpp)
        target_include_directories(aerodyn_mode2_response_test
            PRIVATE src external/controlSystems/include ${GLM_INCLUDE_DIR})
        target_link_libraries(aerodyn_mode2_response_test
            PRIVATE control_systems dynamic_models PkgConfig::JSONC)
        target_compile_definitions(aerodyn_mode2_response_test PRIVATE
            ASR_DEFAULT_AIRCRAFT_SPEC="${CMAKE_CURRENT_SOURCE_DIR}/config/reference-quad.aircraft.json"
            ASR_DEFAULT_COEFFICIENT_BUNDLE="${CMAKE_CURRENT_SOURCE_DIR}/config/reference-quad.coefficients.json")
        add_test(NAME aerodyn_mode2_response_test COMMAND aerodyn_mode2_response_test)

        add_executable(aerodyn_mission_sequence_test
            tests/test_mission_sequence.cpp
            src/modules/mission_control.cpp
            src/modules/flight_control.cpp
            src/modules/quadcopter_dynamics.cpp
            src/config/aircraft_config.cpp)
        target_include_directories(aerodyn_mission_sequence_test
            PRIVATE src external/controlSystems/include ${GLM_INCLUDE_DIR})
        target_link_libraries(aerodyn_mission_sequence_test
            PRIVATE control_systems dynamic_models PkgConfig::JSONC)
        target_compile_definitions(aerodyn_mission_sequence_test PRIVATE
            ASR_DEFAULT_AIRCRAFT_SPEC="${CMAKE_CURRENT_SOURCE_DIR}/config/reference-quad.aircraft.json"
            ASR_DEFAULT_COEFFICIENT_BUNDLE="${CMAKE_CURRENT_SOURCE_DIR}/config/reference-quad.coefficients.json")
        add_test(NAME aerodyn_mission_sequence_test COMMAND aerodyn_mission_sequence_test)
    else()
        message(STATUS "GLM not found; skipping plant and flight-control integration tests")
    endif()

    add_executable(aerodyn_aircraft_contract_test
        tests/test_aircraft_config.cpp
        src/config/aircraft_config.cpp)
    target_include_directories(aerodyn_aircraft_contract_test PRIVATE src)
    target_link_libraries(aerodyn_aircraft_contract_test PRIVATE PkgConfig::JSONC)
    target_compile_definitions(aerodyn_aircraft_contract_test PRIVATE
        ASR_DEFAULT_AIRCRAFT_SPEC="${CMAKE_CURRENT_SOURCE_DIR}/config/reference-quad.aircraft.json"
        ASR_DEFAULT_COEFFICIENT_BUNDLE="${CMAKE_CURRENT_SOURCE_DIR}/config/reference-quad.coefficients.json")
    add_test(NAME aerodyn_aircraft_contract_test COMMAND aerodyn_aircraft_contract_test)

    add_executable(aerodyn_generated_mesh_test
        tests/test_obj_mesh.cpp
        src/render/obj_mesh.cpp)
    target_include_directories(aerodyn_generated_mesh_test PRIVATE src)
    target_compile_definitions(aerodyn_generated_mesh_test PRIVATE
        ASR_DEFAULT_AIRCRAFT_MESH="${CMAKE_CURRENT_SOURCE_DIR}/assets/generated/asr-reference-quad.obj")
    add_test(NAME aerodyn_generated_mesh_test COMMAND aerodyn_generated_mesh_test)
endif()

# If attitude is set up as an imported or interface library,
# make sure to link properly and ensure that INTERFACE include dirs are set.
# Copy shaders to the build directory
if(BUILD_AERODYN_GUI)
    file(COPY shaders/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/shaders)
endif()
