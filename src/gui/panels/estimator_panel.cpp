#include "gui/panels/estimator_panel.h"

#include <glm/glm.hpp>

#include "core/simulation_state.h"
#include "render/camera.h"
#include "attitude/attitude_utils.h"

#include "imgui.h"

void EstimatorPanel::draw(SimulationState& state, Camera& camera) {
    (void)camera;

    double roll_true = state.euler.roll;
    double pitch_true = state.euler.pitch;
    double yaw_true = state.euler.yaw;

    double roll_est = state.estimator.euler.roll;
    double pitch_est = state.estimator.euler.pitch;
    double yaw_est = state.estimator.euler.yaw;

    if (ImGui::Begin(name(), nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("True vs Estimated Orientation (deg)");
        ImGui::Separator();
        ImGui::Text("Roll:   %.2f / %.2f (err %.2f)", rad2deg(roll_true), rad2deg(roll_est), rad2deg(roll_est - roll_true));
        ImGui::Text("Pitch:  %.2f / %.2f (err %.2f)", rad2deg(pitch_true), rad2deg(pitch_est), rad2deg(pitch_est - pitch_true));
        ImGui::Text("Yaw:    %.2f / %.2f (err %.2f)", rad2deg(yaw_true), rad2deg(yaw_est), rad2deg(yaw_est - yaw_true));

        ImGui::Separator();
        ImGui::Text("Estimator Quaternion:");
        ImGui::Text("[%.4f, %.4f, %.4f, %.4f]",
                    state.estimator.quaternion[0],
                    state.estimator.quaternion[1],
                    state.estimator.quaternion[2],
                    state.estimator.quaternion[3]);

        ImGui::Separator();
        ImGui::Text("Sensor Inputs");
        ImGui::Text("Gyro (rad/s): %.3f %.3f %.3f",
                    state.sensor.gyro_rad_s.x,
                    state.sensor.gyro_rad_s.y,
                    state.sensor.gyro_rad_s.z);
        ImGui::Text("Accel (m/s^2): %.3f %.3f %.3f",
                    state.sensor.accel_mps2.x,
                    state.sensor.accel_mps2.y,
                    state.sensor.accel_mps2.z);
    }
    ImGui::End();
}

