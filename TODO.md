# AeroDynControlRig TODO List

**Last Updated:** 2025-10-25

This roadmap tracks the implementation of telemetry graphs, scene polish, and full flight control integration based on the vision mocks in `vision/state_estimation_view/`.

---

## Phase 1: Data Infrastructure - Quaternion History Buffers

- [ ] Add ring buffer to SimulationState for timestamped quaternion history (configurable depth, e.g., 1000 samples)
- [ ] Add ring buffer to SimulationState for timestamped Euler angle history
- [ ] Add ring buffer for angular rates (gyro) history
- [ ] Add ring buffer for rotor dynamics (RPM per motor, thrust, power consumption, temperature)
- [ ] Add ring buffer for state estimation data (position, velocity, accelerometer, gyroscope, magnetometer)
- [ ] Implement configurable sampling rate tied to fixed/real dt to keep data density predictable

---

## Phase 2: Plotting Infrastructure - ImPlot Integration

- [ ] Add ImPlot library as git submodule to external/
- [ ] Integrate ImPlot into CMakeLists.txt build system
- [ ] Create plotting utility wrapper (src/gui/widgets/plot_widget.h) for common plot configurations

---

## Phase 3: Telemetry Panel - Roll/Pitch/Yaw Graphs (Vision Mock 2)

- [ ] Add Roll Angle (Raw vs Estimated) plot to Estimator Panel matching vision mock
- [ ] Add Pitch Angle (Raw vs Estimated) plot to Estimator Panel
- [ ] Add Yaw Angle (Raw vs Estimated) plot to Estimator Panel
- [ ] Add X-Axis Acceleration plot with 60s time window
- [ ] Add time window controls (10s / 30s / 60s) for all plots
- [ ] Add pause/resume button for plot updates
- [ ] Add clear history button for plots

---

## Phase 4: Rotor Dynamics Panel - Thrust/RPM/Power/Temp Graphs (Vision Mock 1)

- [ ] Create Rotor Analysis panel with sidebar navigation (Rotor 1/2/3/4 selection)
- [ ] Add Thrust vs. Time plot with cyan gradient fill matching vision
- [ ] Add RPM vs. Time plot with orange/yellow gradient fill
- [ ] Add Power Consumption vs. Time plot with cyan gradient
- [ ] Add Temperature vs. Time plot with orange gradient
- [ ] Add Raw Data Log table showing Timestamp, Thrust, RPM, Voltage columns
- [ ] Add playback controls (play/pause/skip) with timeline scrubber
- [ ] Add Export Data button (CSV export functionality)

---

## Phase 5: Scene Rendering Polish - Matching Main Dashboard Vision

- [ ] Implement PBR/Phong shader with lighting (directional key light + rim light)
- [ ] Add gradient skybox/background matching teal-to-purple glow in vision
- [ ] Add floor plane with grid for spatial reference
- [ ] Implement simple blob shadow or planar shadow map for drone
- [ ] Add vignette post-process effect to renderSceneToTexture
- [ ] Add optional film grain post-process for cinematic feel
- [ ] Replace wireframe cube with solid mesh + emissive edge lines (or import higher-fidelity model)

---

## Phase 6: UI Polish - Matching Dashboard Aesthetic

- [ ] Update card styling to match vision (darker backgrounds, border colors)
- [ ] Add large metric displays (e.g., '1500 RPM', '75W') with change indicators (+2%, -1%)
- [ ] Implement per-rotor bar chart visualization (R1/R2/R3/R4) for RPM
- [ ] Add status badges (Connected, Autonomous mode, Converged/Diverged/Installing)
- [ ] Add navigation tabs (Dashboard, Raw Data, Error Analysis, Settings)

---

## Phase 7: Dynamics + Physics Integration

- [ ] Resurrect FirstOrderDynamicsModule and integrate with new plotting infrastructure
- [ ] Resurrect RotorTelemetryModule and connect to rotor plotting buffers
- [ ] Extend SimulationState with acceleration, force, and commanded state slots
- [ ] Add feature flag for full quadrotor equations of motion (EoMs)
- [ ] Wire physics outputs (thrust, forces, accelerations) into plotting buffers

---

## Phase 8: State Estimation + INS Pipeline

- [ ] Integrate stateEstimation library (Kalman filter, complementary filter)
- [ ] Add ComplementaryEstimatorModule back to module pipeline
- [ ] Add innovation plots (measurement - prediction) for Kalman filter
- [ ] Add covariance plots showing estimation uncertainty
- [ ] Add raw vs. estimated comparison plots (matching vision mock 2)

---

## Phase 9: Controller Integration

- [ ] Integrate controlSystems library (PID, LQR, MPC)
- [ ] Add PID gain tuning sliders to Control Panel (P, I, D)
- [ ] Add controller mode selector (Manual, PID, LQR, MPC, H-Infinity)
- [ ] Add commanded vs. actual state plots for control analysis

---

## Phase 10: Data Export and Logging

- [ ] Implement CSV export for all telemetry buffers
- [ ] Add binary log format for efficient storage (optional)
- [ ] Add log replay functionality to load and visualize historical data

---

## Estimated Timeline

- **Phase 1-2:** 1 week (data infrastructure + ImPlot)
- **Phase 3-4:** 2 weeks (telemetry graphs)
- **Phase 5-6:** 1-2 weeks (scene polish + UI)
- **Phase 7-9:** 2-3 weeks (dynamics + estimation + control)
- **Phase 10:** 1 week (export/logging)

**Total:** 6-8 weeks for full implementation
