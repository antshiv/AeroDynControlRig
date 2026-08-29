# Mode 2 Flight Controls

AeroDyn polls Logitech F310 and Dual Action controllers through GLFW's standardized gamepad API. It does not read `/dev/input` directly, so keyboard, UI, and gamepad events remain under one desktop input boundary. Linux may expose product `046d:c216` as a DirectInput Dual Action; AeroDyn installs the corresponding maintained SDL mapping before consuming any buttons.

## Mode 2 Mapping

| Input | Simulation command |
|---|---|
| Left stick left/right | Rotate in place by changing heading; it does not command travel |
| Left stick up/down | Climb/descend speed; centred commands zero vertical speed |
| Right stick left/right | Fly left/right; centred brakes lateral motion |
| Right stick up/down | Fly forward/back; centred brakes forward motion |
| A | Take off from the ground |
| B | Land while taking off or flying |
| X | Pause or resume an active flight |
| Y | Recenter the aircraft in the 3D viewport |
| Back | Emergency stop, pause, clear motors, and require a reset |
| Start | Reset the SIL aircraft, controller, and telemetry |
| Guide | Show or hide the in-application guide when exposed by GLFW |

The physical **MODE** button is different from the Guide button. On the F310 it may switch the D-pad and left-stick mapping in hardware, so Linux/GLFW may not report it as an independent button. The panel still exposes the resulting axis and D-pad state, making that mode change visible.

The basic profile deliberately gives each button one meaning. Pressing A while airborne, B while grounded, or X without an active flight is ignored with a visible status message rather than triggering a second command.

The input panel highlights active sticks and face buttons. It also displays raw normalized axes so mapping, centring, and hardware-mode faults are visible.
It records a monotonic action number and the last normalized physical action, so repeated-button behavior can be diagnosed independently of controller-status messages.

The `Camera orbit`, `Camera pan`, and `Camera zoom` controls under the OpenGL scene move only the viewpoint. They never command the aircraft. Dashboard keyboard letters are intentionally disconnected from both camera and flight motion to avoid ambiguous control ownership.

## Training Assist And Advanced Mode

Training assist is the default. The right stick commands body-relative horizontal velocity, the left vertical stick commands climb/descent velocity, and releasing the sticks commands zero velocity. A proportional outer loop converts velocity error into bounded roll, pitch, and collective targets before the existing attitude PID and mixer.

Advanced flight mode exposes the underlying attitude/collective behavior directly. Releasing the right stick levels the attitude but does not brake accumulated horizontal velocity. This mode is useful for studying the plant and controller, but it is intentionally harder to fly.

The separate **Advanced trim and trigger controls** checkbox enables D-pad trim, LB/RB authority changes, trigger-based vertical commands, and stick-click shortcuts. These inputs are disabled by default so hardware mode changes or accidental trigger values cannot alter the basic flight contract.

The application opens grounded, paused, and with all virtual motors stopped. Set the altitude and press **A** (physical button 2 on a Dual Action) or click **Take off** to run the checked climb and hover transition. Press **B** (physical button 3) or click **Land** to run the checked descent and return to a zero-motor grounded state. Use **X** to pause or resume an active flight. The sticks command flight only after takeoff begins; they cannot move a grounded aircraft. The **Fit aircraft** viewport button and Y button restore the inspection view at any time.

## Safety Boundary

Pressing A does not blindly activate control. All four axes must be within the enable deadzone and the controller must not report the known all-axes-at-minus-one state. Disconnecting the controller disables pilot control. Invalid input, controller timing, PID output, or mixer output fails closed.

The current coefficient bundle is marked `simulation_only`. Joystick enablement is not motor arming and cannot approve physical flight. ASR-FC hardware requires separate arming, watchdog, command-timeout, HIL, restrained-motor, and flight-test gates.

## Runtime Path

```text
Mode 2 gamepad / GLFW
    -> normalized pilot command
    -> grounded / takeoff / flight / landing mission contract
    -> velocity-assist outer loop (default)
    -> quaternion attitude setpoint + yaw-rate target + collective
    -> controlSystems PID (40 ms declared cadence)
    -> geometry-derived controlSystems mixer
    -> four rotor angular velocities
    -> checked dynamic_models RK4 plant
    -> OpenGL aircraft + telemetry + controller infographic
```
