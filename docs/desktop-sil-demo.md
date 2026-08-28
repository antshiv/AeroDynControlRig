# Desktop SIL Demo

The desktop demo is a closed software loop, not a scripted animation:

```text
Logitech F310
    -> controlSystems PID
    -> geometry-derived four-rotor mixer
    -> checked dynamic_models RK4 plant
    -> OpenGL aircraft and telemetry
```

The aircraft contract in `config/reference-quad.aircraft.json` supplies mass, inertia, gravity, rotor positions, spin directions, thrust coefficients, torque coefficients, and speed limits. The controller bundle is revision-linked to that aircraft contract. Invalid or mismatched artifacts stop initialization rather than falling back to placeholder values.

## Running The Demo

1. Connect a Logitech F310 in XInput mode.
2. Start `AeroDynControlRig` and open the `F310 Flight Controls` tab.
3. Center both sticks and confirm that the controller, mixer, and RK4 readiness gates pass.
4. Press `A` or select `Enable + run desktop SIL`.
5. Use the left stick for collective thrust and yaw. Use the right stick for pitch and roll.
6. Press the right stick (`R3`) to fit the aircraft in the scene.
7. Press `BACK` to stop control, reset the PID, and clear every virtual motor command.

The remaining mapped controls are shown directly on the in-application F310 diagram. Motor commands are virtual. This mode does not energize hardware.

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

Desktop SIL currently validates joystick mapping, controller timing, mixing, saturation, rigid-body propagation, and visualization. It does not yet model propulsion transients, ESC current loops, battery sag, sensor noise, CEVA estimation, wind, ground contact, navigation, or mission logic.

The mode selector reserves two later paths:

- `nRF5340 HIL` will send simulated sensor frames to physical ASR-FC firmware and return four virtual motor commands before advancing the plant.
- `Live CEVA replay` will drive the visualization and diagnostics from captured FSM300 output.

Both remain disabled until their transports and timing contracts are implemented and tested.
