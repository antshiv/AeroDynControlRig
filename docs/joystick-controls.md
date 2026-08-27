# Logitech F310 Flight Controls

AeroDyn polls the Logitech F310 through GLFW's standardized gamepad API. It does not read `/dev/input` directly, so keyboard, UI, and gamepad events remain under one desktop input boundary.

## Mode 2 Mapping

| Input | Simulation command |
|---|---|
| Left stick X | Yaw-rate target |
| Left stick Y | Collective thrust around hover |
| Right stick X | Roll-attitude target, limited to 25 degrees |
| Right stick Y | Pitch-attitude target, limited to 25 degrees |
| A | Enable or disable closed-loop SIL control |
| B | Pause or resume simulation |
| X | Level roll/pitch and retain current heading |
| Y | Show or hide the in-application guide |
| Back | Disable control and clear PID state |
| D-pad | Apply one-degree roll/pitch trim steps |
| LB / RB | Reduce/increase stick command authority from 25% to 100% |
| LT / RT | Reduce/increase collective around the left-stick command |
| Left-stick click | Clear trim and level the attitude target |
| Right-stick click | Reset the 3D inspection camera |
| Start | Reset the SIL aircraft, controller, and telemetry |
| Guide | Show or hide the in-application guide when exposed by GLFW |

The physical **MODE** button is different from the Guide button. On the F310 it may switch the D-pad and left-stick mapping in hardware, so Linux/GLFW may not report it as an independent button. The panel still exposes the resulting axis and D-pad state, making that mode change visible.

The input panel highlights active sticks and face buttons. It also displays raw normalized axes so mapping, centring, and hardware-mode faults are visible.

## Safety Boundary

Pressing A does not blindly activate control. All four axes must be within the enable deadzone and the F310 must not report the known all-axes-at-minus-one state. Disconnecting the controller disables pilot control. Invalid input, controller timing, PID output, or mixer output fails closed.

The current coefficient bundle is marked `simulation_only`. Joystick enablement is not motor arming and cannot approve physical flight. ASR-FC hardware requires separate arming, watchdog, command-timeout, HIL, restrained-motor, and flight-test gates.

## Runtime Path

```text
F310 / GLFW
    -> normalized pilot command
    -> quaternion attitude setpoint + yaw-rate target + collective
    -> controlSystems PID (40 ms declared cadence)
    -> geometry-derived controlSystems mixer
    -> four rotor angular velocities
    -> checked dynamic_models RK4 plant
    -> OpenGL aircraft + telemetry + controller infographic
```
