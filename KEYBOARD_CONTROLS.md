# Keyboard Controls

**Last Updated:** 2025-10-25

## Rotation Mode Toggle

Press **M** to switch between two rotation modes:

### Mode 1: AUTOMATIC (Default) - Continuous Angular Rates
- Simulates real drone flight with angular velocity
- Hold keys to continuously spin
- Press **Space** to stop all rotation

### Mode 2: MANUAL - Discrete Step Rotation
- For testing/debugging quaternions
- Each key press rotates by a fixed amount
- No automatic spinning

---

## AUTOMATIC Mode - Continuous Angular Rates (Default)

Hold keys to add angular velocity (like flying a drone):

| Key | Action | Axis |
|-----|--------|------|
| **Q** (hold) | Roll left (continuous) | X-axis |
| **E** (hold) | Roll right (continuous) | X-axis |
| **↑ / I** (hold) | Pitch up (continuous) | Y-axis |
| **↓ / K** (hold) | Pitch down (continuous) | Y-axis |
| **← / J** (hold) | Yaw left (continuous) | Z-axis |
| **→ / L** (hold) | Yaw right (continuous) | Z-axis |
| **Space** | Stop all rotation (zero rates) | All |

**Behavior:**
- Acceleration: 180°/s² when key held
- Keeps spinning even after you release the key (like real angular momentum)
- Press **Space** to instantly stop all rotation

---

## MANUAL Mode - Discrete Step Rotation

Each key press rotates by a fixed amount (no continuous spinning):

### Normal Steps (5° per press)

| Key | Action |
|-----|--------|
| **W** | Pitch up (+5°) |
| **S** | Pitch down (-5°) |
| **A** | Roll left (+5°) |
| **D** | Roll right (-5°) |
| **Q** | Yaw left (+5°) |
| **E** | Yaw right (-5°) |
| **R** | Reset to identity (no rotation) |

### Fine Steps (1° per press)

Hold **Shift** + rotation key for 1-degree increments:

| Key | Action |
|-----|--------|
| **Shift + W** | Pitch up (+1°) |
| **Shift + S** | Pitch down (-1°) |
| **Shift + A** | Roll left (+1°) |
| **Shift + D** | Roll right (-1°) |
| **Shift + Q** | Yaw left (+1°) |
| **Shift + E** | Yaw right (-1°) |

**Behavior:**
- Cube rotates ONLY when you press a key
- Stays perfectly still between key presses
- Angular rates are kept at zero
- Perfect for testing specific quaternion orientations

## Camera Controls

| Key | Action |
|-----|--------|
| **Mouse Drag** | Orbit camera around origin |
| **Scroll Wheel** | Zoom in/out |

## Application Controls

| Key | Action |
|-----|--------|
| **M** | Toggle between AUTOMATIC and MANUAL rotation modes |
| **ESC** | Exit application |
| **Space** | Stop all rotation (AUTOMATIC mode only) |

---

## Implementation Details

### Rotation Modes

#### AUTOMATIC Mode (default, `manual_rotation_mode = false`)
- **Angular rate integration:** Body rates are integrated each frame by QuaternionDemoModule
- **Acceleration model:** 180°/s² when keys held (simulates real physics)
- **Momentum:** Rotation continues after key release (like real angular momentum)
- **Stop:** Space bar zeros all angular rates instantly
- **Keys:** Arrow keys, I/K/J/L, Q/E (continuous hold)

#### MANUAL Mode (`manual_rotation_mode = true`)
- **Direct quaternion update:** Euler → Quaternion conversion on each key press
- **Zero angular rates:** Rates forced to 0.0 to prevent drift
- **Discrete steps:** 5° default, 1° with Shift modifier
- **No momentum:** Rotation only when key pressed
- **Keys:** W/A/S/D/Q/E/R (discrete presses)

### Hardware Mapping

This control scheme mirrors real flight controller telemetry:
- **W/S** (or ↑/↓) = Pitch (elevator stick on RC controller)
- **A/D** = Roll (aileron stick on RC controller)
- **Q/E** (or ←/→) = Yaw (rudder stick on RC controller)

### Technical Notes

- Quaternions are stored as `{w, x, y, z}` (scalar-first convention)
- Euler conversions use `attitudeMathLibrary` functions
- Euler order: Yaw-Pitch-Roll (ZYX)
- Face culling is enabled (back-faces are not rendered)
- Toggle state stored in `SimulationState.control.manual_rotation_mode`
