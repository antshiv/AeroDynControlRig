#include "gui/panels/mathematical_model_panel.h"

#include <cmath>

#include "core/simulation_state.h"
#include "imgui.h"
#include "render/camera.h"

namespace {
void matrixTable(const char* id, const double* values,
                 std::size_t rows, std::size_t columns,
                 const char* const* row_names,
                 const char* const* column_names) {
    if (!ImGui::BeginTable(id, static_cast<int>(columns + 1u),
                           ImGuiTableFlags_Borders |
                           ImGuiTableFlags_RowBg |
                           ImGuiTableFlags_ScrollX,
                           ImVec2(0.0f, 260.0f))) {
        return;
    }
    ImGui::TableSetupColumn("row", ImGuiTableColumnFlags_WidthFixed, 55.0f);
    for (std::size_t column = 0u; column < columns; ++column) {
        ImGui::TableSetupColumn(column_names[column],
                                ImGuiTableColumnFlags_WidthFixed, 62.0f);
    }
    ImGui::TableHeadersRow();
    for (std::size_t row = 0u; row < rows; ++row) {
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted(row_names[row]);
        for (std::size_t column = 0u; column < columns; ++column) {
            ImGui::TableSetColumnIndex(static_cast<int>(column + 1u));
            const double value = values[row * columns + column];
            if (std::abs(value) < 1.0e-12) {
                ImGui::TextDisabled("0");
            } else {
                ImGui::Text("%.3g", value);
            }
        }
    }
    ImGui::EndTable();
}
}

void MathematicalModelPanel::draw(SimulationState& state, Camera& camera) {
    (void)camera;
    ImGui::SetNextWindowSize(ImVec2(720.0f, 720.0f), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin(name())) {
        ImGui::End();
        return;
    }

    const auto& model = state.mathematical_model;
    ImGui::TextColored(model.valid ? ImVec4(0.25f, 0.9f, 0.55f, 1.0f)
                                   : ImVec4(1.0f, 0.35f, 0.30f, 1.0f),
                       "%s", model.valid ? "MODEL CONTRACT LOADED" :
                                             "MODEL CONTRACT INVALID");
    ImGui::TextUnformatted(model.model_name.c_str());
    ImGui::TextWrapped(
        "The OpenGL aircraft is advanced by the nonlinear model below. "
        "The A and B matrices are a separate hover linearization for analysis "
        "and controller reasoning; they do not replace the RK4 plant.");

    ImGui::SeparatorText("Nonlinear plant used by the simulator");
    ImGui::TextUnformatted("p_dot = v");
    ImGui::TextUnformatted("m v_dot = R(q) sum_i(kT_i omega_i^2 a_i) + m g_N");
    ImGui::TextUnformatted("q_dot = 0.5 q tensor [0, omega]");
    ImGui::TextUnformatted("I omega_dot = tau - omega x (I omega)");
    ImGui::TextUnformatted("T_i = kT_i omega_i^2,  Q_i = s_i kQ_i omega_i^2");
    ImGui::TextDisabled("Integrator: checked transactional RK4 / NED inertial / FRD body");

    ImGui::SeparatorText("Loaded aircraft numbers");
    ImGui::Text("Mass %.3f kg  |  gravity %.5f m/s^2",
                model.mass_kg, model.gravity_m_s2);
    ImGui::Text("Inertia diagonal [%.4f, %.4f, %.4f] kg m^2",
                model.inertia_diagonal[0], model.inertia_diagonal[1],
                model.inertia_diagonal[2]);
    ImGui::Text("Calculated hover speed %.2f rad/s (%.0f RPM)",
                model.hover_omega_rad_s,
                model.hover_omega_rad_s * 60.0 / 6.28318530717958647692);

    ImGui::SeparatorText("Hover state-space model");
    ImGui::TextUnformatted("x_dot = A x + B u");
    ImGui::TextWrapped("x = [p_n p_e p_d v_n v_e v_d phi theta psi p q r]");
    ImGui::TextWrapped("u = [dT tau_x tau_y tau_z]");
    ImGui::TextDisabled("Linearization: %s", model.linearization.c_str());

    if (ImGui::CollapsingHeader("A matrix (12 x 12)")) {
        matrixTable("A_matrix", model.A.data(), model.kStateCount,
                    model.kStateCount, model.state_names.data(),
                    model.state_names.data());
    }
    if (ImGui::CollapsingHeader("B matrix (12 x 4)")) {
        matrixTable("B_matrix", model.B.data(), model.kStateCount,
                    model.kInputCount, model.state_names.data(),
                    model.input_names.data());
    }

    ImGui::SeparatorText("Local transfer-function view");
    ImGui::Text("phi(s) / tau_x(s) = 1 / (%.4f s^2)",
                model.inertia_diagonal[0]);
    ImGui::Text("theta(s) / tau_y(s) = 1 / (%.4f s^2)",
                model.inertia_diagonal[1]);
    ImGui::Text("psi(s) / tau_z(s) = 1 / (%.4f s^2)",
                model.inertia_diagonal[2]);
    ImGui::Text("p_d(s) / dT(s) = -1 / (%.3f s^2)", model.mass_kg);
    ImGui::TextDisabled(
        "These transfer functions are valid near hover before actuator, "
        "drag, saturation, sensor, and estimator dynamics are added.");
    ImGui::End();
}
