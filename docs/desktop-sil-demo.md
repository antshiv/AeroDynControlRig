# Desktop SIL Demo

The desktop demo is a closed software loop, not a scripted animation:

```text
Mode 2 gamepad
    -> controlSystems PID
    -> geometry-derived four-rotor mixer
    -> checked dynamic_models RK4 plant
    -> OpenGL aircraft and telemetry
```

The aircraft contract in `config/reference-quad.aircraft.json` supplies mass, inertia, gravity, rotor positions, spin directions, thrust coefficients, torque coefficients, and speed limits. The controller bundle is revision-linked to that aircraft contract. Invalid or mismatched artifacts stop initialization rather than falling back to placeholder values.

## Running The Demo

1. Connect a Logitech F310 or Dual Action controller. AeroDyn installs the maintained Dual Action mapping when Linux exposes product `046d:c216` in DirectInput mode.
2. Start `AeroDynControlRig` and open the `Mode 2 Flight Controls` tab.
3. Center both sticks and confirm that the controller, mixer, and RK4 readiness gates pass.
4. Confirm the controller, centred-stick, mixer, and RK4 readiness gates pass. `A` remains an optional run/resume control for the simulation clock.
5. Select the takeoff altitude and press `Take off`. Mission control enables the SIL path and owns the bounded climb until the altitude and vertical-speed gates pass.
6. Push the left stick up/down to climb/descend and left/right to rotate in place. Push the right stick up/down to move forward/back and left/right to move sideways.
7. Press `Land` to return through the bounded descent and ground-contact gates.
8. Press the right stick (`R3`) to fit the aircraft in the scene.
9. Press `BACK` to stop control, reset the PID, clear every virtual motor command, and latch the emergency-stop phase until reset.

The face buttons are deliberately idempotent: `A` always runs or resumes, `B` always pauses, `X` always levels the attitude target, and `Y` always recenters the aircraft. Repeated presses do not switch a button to a different operation.

The remaining mapped controls are shown directly on the in-application controller diagram. A Dual Action is rendered with its physical `1` through `4` labels while the input boundary normalizes it to standard gamepad actions. Motor commands are virtual. This mode does not energize hardware.

Both sticks are spring-centered, unlike a traditional RC transmitter. In the default training-assist mode, AeroDyn interprets the left vertical axis as desired climb/descent speed rather than absolute throttle. The right stick requests horizontal speed, and centring it asks the controller to brake. Advanced mode exposes direct collective and attitude commands for controller study. The default 35% expo softens motion near stick centre without removing full travel; the value can be adjusted in the controller panel.

## Mathematical Plant

The renderer follows a nonlinear Newton-Euler rigid-body model in NED inertial and FRD body coordinates:

```text
p_dot = v
m v_dot = R(q) sum_i(kT_i omega_i^2 a_i) + m g_N
q_dot = 0.5 q tensor [0, omega]
I omega_dot = tau - omega x (I omega)
```

Each accepted frame is integrated through the checked transactional RK4 path. Invalid time steps or failed numerical stages pause the simulation before committing a partial state.

The `Aircraft Mathematical Model` tab also publishes a 12-state hover linearization:

```text
x = [p_n p_e p_d v_n v_e v_d phi theta psi p q r]
u = [dT tau_x tau_y tau_z]
x_dot = A x + B u
```

The A and B matrices and local transfer functions are analysis views of the loaded aircraft. They do not replace the nonlinear plant. Their small-angle hover assumptions are displayed beside them.

## Current Boundary

Desktop SIL currently validates joystick mapping, mission transitions, controller timing, mixing, saturation, rigid-body propagation, a rigid level ground plane, and visualization. The repeatable baseline is `grounded -> takeoff -> hover/flight -> landing -> grounded`. It does not yet model landing-gear compliance, propulsion transients, ESC current loops, battery sag, sensor noise, CEVA estimation, wind, navigation, or autonomous mission planning.

The scene follows the aircraft by default, so takeoff and translation do not silently move it outside the camera frustum. Disable **Follow** to inspect motion against a fixed world-space viewpoint. **Fit aircraft** restores a deterministic orbit and framing.

CI executes `AeroDynControlRig --render-check <artifact.ppm>` under Xvfb. The check renders a background-only baseline and an aircraft frame through the same off-screen framebuffer, then requires a bounded, non-trivial changed-pixel region and no OpenGL error. This proves visible geometry rather than merely proving that the process stayed alive.

The mode selector reserves two later paths:

- `nRF5340 HIL` will send simulated sensor frames to physical ASR-FC firmware and return four virtual motor commands before advancing the plant.
- `Live CEVA replay` will drive the visualization and diagnostics from captured FSM300 output.

`nRF5340 HIL` becomes available only when `ASR_FC_HIL_DEVICE` names a serial
endpoint that opens successfully. The host sends the checked ASR-FC wire format,
waits for the matching session and sequence, and applies only a fault-free MCU
response. A timeout, corrupt frame, stale identity, controller rejection, or
fault flag pauses the plant and clears all four virtual motor commands. The
firmware still contains no PWM or physical motor-output implementation.

`Live CEVA replay` remains disabled until its replay timing contract is connected.
