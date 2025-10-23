#include "gui/panels/dynamics_panel.h"

#include <vector>

#include "core/simulation_state.h"
#include "render/camera.h"
#include "imgui.h"

void DynamicsPanel::appendSample(double time, double input, double output) {
    if (time < last_recorded_time_) {
        output_history_.clear();
        input_history_.clear();
        last_recorded_time_ = 0.0;
    }

    last_recorded_time_ = time;

    output_history_.emplace_back(static_cast<float>(output));
    input_history_.emplace_back(static_cast<float>(input));

    while (output_history_.size() > kMaxSamples) {
        output_history_.pop_front();
    }
    while (input_history_.size() > kMaxSamples) {
        input_history_.pop_front();
    }
}

void DynamicsPanel::draw(SimulationState& state, Camera& camera) {
    (void)camera;

    appendSample(state.time_seconds,
                 state.dynamics_state.input,
                 state.dynamics_state.output);

    if (ImGui::Begin(name(), nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("First-order response");

        ImGui::Checkbox("Use Sine Input", &state.dynamics_config.use_sine);
        if (state.dynamics_config.use_sine) {
            float freq = static_cast<float>(state.dynamics_config.sine_frequency_hz);
            if (ImGui::SliderFloat("Sine Frequency (Hz)", &freq, 0.1f, 5.0f)) {
                state.dynamics_config.sine_frequency_hz = freq;
            }
        } else {
            float target = static_cast<float>(state.dynamics_config.input_target);
            if (ImGui::SliderFloat("Step Input", &target, -2.0f, 2.0f)) {
                state.dynamics_config.input_target = target;
            }
        }

        float tau = static_cast<float>(state.dynamics_config.time_constant);
        if (ImGui::SliderFloat("Time Constant (s)", &tau, 0.05f, 10.0f, "%.2f", ImGuiSliderFlags_Logarithmic)) {
            state.dynamics_config.time_constant = tau;
        }

        float gain = static_cast<float>(state.dynamics_config.gain);
        if (ImGui::SliderFloat("Gain", &gain, 0.1f, 5.0f)) {
            state.dynamics_config.gain = gain;
        }

        static std::vector<float> output_buffer;
        static std::vector<float> input_buffer;
        output_buffer.assign(output_history_.begin(), output_history_.end());
        input_buffer.assign(input_history_.begin(), input_history_.end());

        if (!output_buffer.empty()) {
            ImGui::PlotLines("Output", output_buffer.data(), static_cast<int>(output_buffer.size()), 0, nullptr, -2.0f, 2.0f, ImVec2(0, 120));
            ImGui::PlotLines("Input", input_buffer.data(), static_cast<int>(input_buffer.size()), 0, nullptr, -2.0f, 2.0f, ImVec2(0, 80));
        } else {
            ImGui::Text("Waiting for samples...");
        }

        ImGui::Text("Current output: %.3f", state.dynamics_state.output);
    }
    ImGui::End();
}
