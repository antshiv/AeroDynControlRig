#include "application.h"
#include "render/renderer.h"
#include "modules/quaternion_demo.h"
#include "modules/first_order_dynamics.h"
#include "modules/sensor_simulator.h"
#include "modules/complementary_estimator.h"
#include "modules/rotor_telemetry.h"
#include "gui/panel_manager.h"
#include "gui/widgets/card.h"
#include "gui/style.h"
#include "gui/panels/control_panel.h"
#include "gui/panels/telemetry_panel.h"
#include "gui/panels/dynamics_panel.h"
#include "gui/panels/estimator_panel.h"
#include "gui/panels/rotor_panel.h"
#include "gui/panels/power_panel.h"
#include "gui/panels/sensor_panel.h"
#include "attitude/euler.h"
#include "attitude/dcm.h"
#include "attitude/attitude_utils.h"
#include <iostream>
#include <algorithm>
#include <string>
#include <cstdio>
#include <cstdint>
#include "imgui.h"
#include "imgui_internal.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>

namespace {
constexpr float kTopNavHeight = 64.0f;
constexpr float kDockspaceMargin = 24.0f;

void DrawTopNavigation() {
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    const ImVec2 nav_pos = viewport->Pos;
    const ImVec2 nav_size = ImVec2(viewport->Size.x, kTopNavHeight);

    ImGui::SetNextWindowPos(nav_pos);
    ImGui::SetNextWindowSize(nav_size);
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(24.0f, 14.0f));
    const ui::Palette& palette = ui::Colors();
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(palette.canvas_bg.x, palette.canvas_bg.y, palette.canvas_bg.z, 0.98f));

    ImGuiWindowFlags nav_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse |
                                 ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar |
                                 ImGuiWindowFlags_NoDocking;

    if (ImGui::Begin("TopNavigation", nullptr, nav_flags)) {
        const ui::FontSet& fonts = ui::Fonts();
        bool pushed_heading = false;
        if (fonts.heading) {
            ImGui::PushFont(fonts.heading);
            pushed_heading = true;
        }

        ImGui::BeginGroup();
        ImGui::PushStyleColor(ImGuiCol_Text, palette.accent_base);
        ImGui::TextUnformatted("DF");
        ImGui::PopStyleColor();
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Text, palette.text_primary);
        ImGui::TextUnformatted("Drone Flight Dynamics");
        ImGui::PopStyleColor();
        ImGui::EndGroup();

        float nav_right_width = 420.0f;
        float available = ImGui::GetContentRegionAvail().x;
        if (available > nav_right_width) {
            ImGui::Dummy(ImVec2(available - nav_right_width, 0.0f));
            ImGui::SameLine();
        }

        ImGui::BeginGroup();
        ImGui::PushStyleColor(ImGuiCol_Text, palette.text_muted);
        ImGui::TextUnformatted("Dashboard");
        ImGui::SameLine(0.0f, 24.0f);
        ImGui::TextUnformatted("Settings");
        ImGui::SameLine(0.0f, 24.0f);
        ImGui::TextUnformatted("Logs");
        ImGui::PopStyleColor();

        ImGui::SameLine(0.0f, 28.0f);
        ui::PushPillButtonStyle(ui::PillStyle::Primary);
        ImGui::Button("Connected");
        ui::PopPillButtonStyle();

        ImGui::SameLine(0.0f, 12.0f);
        ui::PushPillButtonStyle(ui::PillStyle::Secondary);
        ImGui::Button("Link");
        ui::PopPillButtonStyle();
        ImGui::EndGroup();

        if (pushed_heading) {
            ImGui::PopFont();
        }
    }
    ImGui::End();

    ImGui::PopStyleColor();
    ImGui::PopStyleVar(3);
}
}


Application::Application() {
    // Constructor code
    window = nullptr;
    fbo = 0;
    renderTexture = 0;
    depthBuffer = 0;
    sceneWidth = 0;
    sceneHeight = 0;
}

Application::~Application() {
    // Destructor code
}

bool Application::init() {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return false;
    }

    // Create GLFW window before any ImGui initialization
    window = glfwCreateWindow(800, 600, "Dynamic Control System Test Rig", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(window);
    std::cout << "Window created successfully: " << window << std::endl;

    // Set GLFW callbacks
    glfwSetWindowUserPointer(window, this);
    glfwSetKeyCallback(window, keyCallback);
    //glfwSetCursorPosCallback(window, [](GLFWwindow* w, double x, double y) {
    //    static_cast<Application*>(glfwGetWindowUserPointer(w))->mouseCallback(w, x, y);
    //});
    glfwSetScrollCallback(window, scrollCallback);

    // Check OpenGL version
    const GLubyte* rendererName = glGetString(GL_RENDERER);
    const GLubyte* version = glGetString(GL_VERSION);
    if (rendererName && version) {
        std::cout << "Renderer: " << rendererName << "\nOpenGL version: " << version << std::endl;
    } else {
        std::cerr << "Failed to retrieve OpenGL context information." << std::endl;
        return false;
    }

    // Initialize ImGui context AFTER the window is created
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    ui::ApplyTheme(ImGui::GetStyle());
    ui::LoadFonts(io);

    // Initialize ImGui backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // Initialize renderers
    if (!renderer.init()) {
        std::cerr << "Renderer initialization failed." << std::endl;
        return false;
    }

    if (!axisRenderer.init()) {
        std::cerr << "Axis renderer initialization failed." << std::endl;
        return false;
    }

    // Set orthographic projection for 2D
    float aspectRatio = 800.0f / 600.0f; // Adjust dynamically if needed
    transform.setOrthographic(-aspectRatio, aspectRatio, -1.0f, 1.0f, -1.0f, 1.0f);

    initializeModules();
    initializePanels();
    lastFrame = glfwGetTime();

    return true;
}

bool Application::running() const {
    //std::cout << "Checking window state. Window pointer: " << window << std::endl;
    if (window) {
        bool shouldClose = glfwWindowShouldClose(window);
        //std::cout << "glfwWindowShouldClose result: " << shouldClose << std::endl;
        return !shouldClose;
    } else {
        std::cerr << "Window is NULL in running()" << std::endl;
        return false;
    }
}

void Application::initializeModules() {
    modules.emplace_back(std::make_unique<QuaternionDemoModule>());
    modules.emplace_back(std::make_unique<FirstOrderDynamicsModule>());
    modules.emplace_back(std::make_unique<SensorSimulatorModule>());
    modules.emplace_back(std::make_unique<ComplementaryEstimatorModule>());
    modules.emplace_back(std::make_unique<RotorTelemetryModule>());
    for (auto& module : modules) {
        module->initialize(simulationState);
    }
    transform.model = simulationState.model_matrix;
}



void Application::tick() {
    float currentFrame = glfwGetTime();
    double real_dt = static_cast<double>(currentFrame - lastFrame);
    lastFrame = currentFrame;

    updateCamera(static_cast<float>(real_dt));

    // Handle body rotation speed adjustments via keyboard.
    glm::dvec3& body_rates = simulationState.angular_rate_deg_per_sec;
    auto adjust_rotation = [&](int key, int axis, double direction) {
        if (glfwGetKey(window, key) == GLFW_PRESS) {
            const double kRotationAccelDegPerSec2 = 180.0;
            body_rates[axis] += direction * kRotationAccelDegPerSec2 * real_dt;
        }
    };

    // Roll (X) via Q/E
    adjust_rotation(GLFW_KEY_Q, 0, 1.0);
    adjust_rotation(GLFW_KEY_E, 0, -1.0);

    // Pitch (Y) via Up/Down arrows
    adjust_rotation(GLFW_KEY_UP, 1, 1.0);
    adjust_rotation(GLFW_KEY_DOWN, 1, -1.0);

    // Yaw (Z) via Left/Right arrows
    adjust_rotation(GLFW_KEY_LEFT, 2, 1.0);
    adjust_rotation(GLFW_KEY_RIGHT, 2, -1.0);

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        body_rates = glm::dvec3(0.0f);
    }

    if (!simulationState.control.paused) {
        double dt = simulationState.control.use_fixed_dt
                        ? simulationState.control.fixed_dt
                        : real_dt * simulationState.control.time_scale;

        simulationState.last_dt = dt;

        if (dt > 0.0) {
            for (auto& module : modules) {
                module->update(dt, simulationState);
            }
        }
    } else {
        simulationState.last_dt = 0.0;
    }

    transform.model = simulationState.model_matrix;
    render3D();
}

void Application::shutdown() {
    destroyRenderTarget();

    // Cleanup Dear ImGui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // Cleanup resources
    //renderer.shutdown();
    glfwDestroyWindow(window);
    glfwTerminate();
}

void Application::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    Application* app = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
    if (!app) {
        std::cerr << "Application instance is null in keyCallback" << std::endl;
        return;
    }

    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureKeyboard) {
        return;
    }

    if (action == GLFW_PRESS && key == GLFW_KEY_ESCAPE) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

void Application::mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    Application* app = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
    if (!app) {
        return;
    }

    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureMouse) {
        return;
    }

    static double lastX = xpos;
    static double lastY = ypos;
    float xOffset = static_cast<float>(xpos - lastX);
    float yOffset = static_cast<float>(lastY - ypos); // Reversed since y goes bottom→top
    lastX = xpos;
    lastY = ypos;

    app->camera.processMouseMovement(xOffset, yOffset);
}

void Application::scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    Application* app = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
    if (!app) {
        return;
    }

    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureMouse) {
        return;
    }

    app->camera.processMouseScroll(static_cast<float>(yoffset));
}

void Application::render3D() {
    transform.model = simulationState.model_matrix;

    // Step 1: Clear the framebuffer
    glClearColor(0.06f, 0.08f, 0.10f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Step 2: Render OpenGL 3D content
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);
    glEnable(GL_DEPTH_TEST);

    // Set view and projection matrices
    transform.view = camera.getViewMatrix();
    transform.projection = camera.getProjectionMatrix(static_cast<float>(width) / height);

    // Step 4: Render ImGui UI
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        const ImVec2 dock_pos = ImVec2(viewport->Pos.x + kDockspaceMargin,
                                       viewport->Pos.y + kTopNavHeight + kDockspaceMargin);
        const ImVec2 dock_size = ImVec2(
            std::max(50.0f, viewport->Size.x - 2.0f * kDockspaceMargin),
            std::max(50.0f, viewport->Size.y - kTopNavHeight - 2.0f * kDockspaceMargin));

        ImGui::SetNextWindowPos(dock_pos);
        ImGui::SetNextWindowSize(dock_size);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGuiWindowFlags host_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
                                     ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                                     ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoBringToFrontOnFocus |
                                     ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBackground;
        ImGui::Begin("DockSpaceHost", nullptr, host_flags);
        ImGui::PopStyleVar(3);
        ImGuiID dockspace_id = ImGui::GetID("MainDockspace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);
        ImGui::End();

        static bool dockspace_initialized = false;
        if (!dockspace_initialized) {
            dockspace_initialized = true;
            ImGui::DockBuilderRemoveNode(dockspace_id);
            ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace | ImGuiDockNodeFlags_PassthruCentralNode);
            ImGui::DockBuilderSetNodeSize(dockspace_id, dock_size);

            ImGuiID dock_root = dockspace_id;
            ImGuiID dock_right;
            ImGuiID dock_left;
            ImGuiID dock_right_bottom;
            ImGuiID dock_left_bottom;
            ImGuiID dock_bottom_left;
            ImGuiID dock_bottom_center;
            ImGuiID dock_bottom_right;

            ImGui::DockBuilderSplitNode(dock_root, ImGuiDir_Right, 0.32f, &dock_right, &dock_left);
            ImGui::DockBuilderSplitNode(dock_left, ImGuiDir_Down, 0.42f, &dock_left_bottom, &dock_left);
            ImGui::DockBuilderSplitNode(dock_left_bottom, ImGuiDir_Right, 0.33f, &dock_bottom_right, &dock_left_bottom);
            ImGui::DockBuilderSplitNode(dock_left_bottom, ImGuiDir_Right, 0.50f, &dock_bottom_center, &dock_bottom_left);
            ImGui::DockBuilderSplitNode(dock_right, ImGuiDir_Down, 0.52f, &dock_right_bottom, &dock_right);

            ImGui::DockBuilderDockWindow("Flight Scene", dock_left);
            ImGui::DockBuilderDockWindow("Rotor Dynamics", dock_right);
            ImGui::DockBuilderDockWindow("Power Monitor", dock_right_bottom);
            ImGui::DockBuilderDockWindow("Estimator", dock_bottom_left);
            ImGui::DockBuilderDockWindow("Control Panel", dock_bottom_center);
            ImGui::DockBuilderDockWindow("Sensor Suite", dock_bottom_right);
            ImGui::DockBuilderDockWindow("Flight Telemetry", dock_bottom_center);
            ImGui::DockBuilderDockWindow("Dynamics", dock_right_bottom);
            ImGui::DockBuilderFinish(dockspace_id);
        }
    }

    DrawTopNavigation();

    ui::CardOptions scene_card;
    scene_card.min_size = ImVec2(640.0f, 420.0f);
    scene_card.allow_scrollbar = false;
    scene_card.background_alpha = 0.0f;
    if (ui::BeginCard("Flight Scene", scene_card, nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar)) {
        ui::CardHeader("3D Drone Visualization");

        const ui::Palette& palette = ui::Colors();
        const ui::FontSet& fonts = ui::Fonts();

        ImVec2 avail = ImGui::GetContentRegionAvail();
        avail.x = std::max(avail.x, 1.0f);
        avail.y = std::max(avail.y, 1.0f);

        const float footer_height = 68.0f;
        ImVec2 viewport_size = ImVec2(avail.x, std::max(120.0f, avail.y - footer_height));
        ImVec2 canvas_pos = ImGui::GetCursorScreenPos();
        ImVec2 footer_pos = ImVec2(canvas_pos.x, canvas_pos.y + viewport_size.y);
        ImVec2 footer_size = ImVec2(avail.x, footer_height);

        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        draw_list->AddRect(canvas_pos,
                           canvas_pos + viewport_size,
                           ImGui::ColorConvertFloat4ToU32(palette.card_border),
                           18.0f,
                           ImDrawFlags_RoundCornersAll,
                           2.0f);

        ImTextureID scene_texture = renderSceneToTexture(viewport_size);
        if (scene_texture) {
            ImGui::Image(scene_texture,
                         viewport_size,
                         ImVec2(0.0f, 1.0f),
                         ImVec2(1.0f, 0.0f));

            char quat_buf[96];
            std::snprintf(
                quat_buf,
                sizeof(quat_buf),
                "Quaternion: %.3f, %.3f, %.3f, %.3f",
                simulationState.quaternion[0],
                simulationState.quaternion[1],
                simulationState.quaternion[2],
                simulationState.quaternion[3]);

            char euler_buf[96];
            std::snprintf(
                euler_buf,
                sizeof(euler_buf),
                "Euler (deg): R %.1f  P %.1f  Y %.1f",
                rad2deg(simulationState.euler.roll),
                rad2deg(simulationState.euler.pitch),
                rad2deg(simulationState.euler.yaw));

            ImVec2 text_pos = canvas_pos + ImVec2(18.0f, 18.0f);
            draw_list->AddText(text_pos,
                               ImGui::ColorConvertFloat4ToU32(palette.text_primary),
                               quat_buf);
            draw_list->AddText(text_pos + ImVec2(0.0f, 20.0f),
                               ImGui::ColorConvertFloat4ToU32(palette.text_muted),
                               euler_buf);

            char rates_buf[96];
            std::snprintf(
                rates_buf,
                sizeof(rates_buf),
                "Body Rate (deg/s): R %.1f  P %.1f  Y %.1f",
                simulationState.angular_rate_deg_per_sec.x,
                simulationState.angular_rate_deg_per_sec.y,
                simulationState.angular_rate_deg_per_sec.z);

            draw_list->AddText(text_pos + ImVec2(0.0f, 40.0f),
                               ImGui::ColorConvertFloat4ToU32(palette.text_muted),
                               rates_buf);
        } else {
            ImGui::Dummy(viewport_size);
            ImGui::SetCursorScreenPos(canvas_pos + ImVec2(18.0f, 18.0f));
            ImGui::PushStyleColor(ImGuiCol_Text, palette.text_muted);
            ImGui::TextUnformatted("Scene renderer unavailable");
            ImGui::PopStyleColor();
            ImGui::SetCursorScreenPos(canvas_pos + ImVec2(0.0f, viewport_size.y));
        }

        ImGui::Dummy(ImVec2(0.0f, 12.0f));

        // Footer control strip
        ImDrawFlags footer_rounding = ImDrawFlags_RoundCornersBottom;
        draw_list->AddRectFilled(footer_pos,
                                 footer_pos + footer_size,
                                 ImGui::ColorConvertFloat4ToU32(ImVec4(palette.card_header.x,
                                                                       palette.card_header.y,
                                                                       palette.card_header.z,
                                                                       0.96f)),
                                 18.0f,
                                 footer_rounding);

        ImGui::SetCursorScreenPos(footer_pos + ImVec2(24.0f, 18.0f));

        auto footer_button = [&](const char* id, const char* label) {
            std::string text = label;
            if (fonts.icon) {
                text = std::string(u8"\ue5d1 ") + label;  // Material rotate icon
            }
            ImGui::PushID(id);
            ui::PushPillButtonStyle(ui::PillStyle::Secondary);
            ImGui::Button(text.c_str(), ImVec2(120.0f, 0.0f));
            ui::PopPillButtonStyle();
            ImGui::PopID();
        };

        footer_button("rotate", "Rotate");
        ImGui::SameLine(0.0f, 18.0f);

        auto footer_button_plain = [&](const char* id, const char* label, const char* icon_code) {
            std::string text = label;
            if (fonts.icon && icon_code) {
                text = std::string(icon_code) + " " + label;
            }
            ImGui::PushID(id);
            ui::PushPillButtonStyle(ui::PillStyle::Secondary);
            ImGui::Button(text.c_str(), ImVec2(120.0f, 0.0f));
            ui::PopPillButtonStyle();
            ImGui::PopID();
        };

        footer_button_plain("pan", "Pan", u8"\ue55d");
        ImGui::SameLine(0.0f, 18.0f);
        footer_button_plain("zoom", "Zoom", u8"\ue8ff");
    }
    ui::EndCard();

    panelManager.drawAll(simulationState, camera);

    // Render ImGui
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // Step 5: Swap buffers and poll events
    glfwSwapBuffers(window);
    glfwPollEvents();
}

void Application::updateCamera(float deltaTime) {
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureKeyboard) {
        return;
    }

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.processKeyboardInput(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.processKeyboardInput(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.processKeyboardInput(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.processKeyboardInput(RIGHT, deltaTime);
}

void Application::initializePanels() {
    panelManager.registerPanel(std::make_unique<ControlPanel>());
    panelManager.registerPanel(std::make_unique<TelemetryPanel>());
    panelManager.registerPanel(std::make_unique<RotorPanel>());
    panelManager.registerPanel(std::make_unique<PowerPanel>());
    panelManager.registerPanel(std::make_unique<SensorPanel>());
    panelManager.registerPanel(std::make_unique<DynamicsPanel>());
    panelManager.registerPanel(std::make_unique<EstimatorPanel>());
}

ImTextureID Application::renderSceneToTexture(const ImVec2& size) {
    int requested_width = std::max(1, static_cast<int>(size.x));
    int requested_height = std::max(1, static_cast<int>(size.y));

    if (!ensureRenderTarget(requested_width, requested_height)) {
        return static_cast<ImTextureID>(0);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glViewport(0, 0, requested_width, requested_height);
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.06f, 0.09f, 0.12f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    Transform scene_transform = transform;
    scene_transform.projection = camera.getProjectionMatrix(static_cast<float>(requested_width) /
                                                            static_cast<float>(requested_height));

    renderer.renderFrame3D(scene_transform);
    axisRenderer.render3D(scene_transform);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    int window_width = 0;
    int window_height = 0;
    glfwGetFramebufferSize(window, &window_width, &window_height);
    glViewport(0, 0, window_width, window_height);

    return static_cast<ImTextureID>(renderTexture);
}

bool Application::ensureRenderTarget(int width, int height) {
    if (width == sceneWidth && height == sceneHeight &&
        fbo != 0 && renderTexture != 0 && depthBuffer != 0) {
        return true;
    }

    destroyRenderTarget();
    sceneWidth = width;
    sceneHeight = height;

    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glGenTextures(1, &renderTexture);
    glBindTexture(GL_TEXTURE_2D, renderTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderTexture, 0);

    glGenRenderbuffers(1, &depthBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);

    bool complete = (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    if (!complete) {
        destroyRenderTarget();
        return false;
    }
    return true;
}

void Application::destroyRenderTarget() {
    if (depthBuffer) {
        glDeleteRenderbuffers(1, &depthBuffer);
        depthBuffer = 0;
    }
    if (renderTexture) {
        glDeleteTextures(1, &renderTexture);
        renderTexture = 0;
    }
    if (fbo) {
        glDeleteFramebuffers(1, &fbo);
        fbo = 0;
    }
    sceneWidth = 0;
    sceneHeight = 0;
}
