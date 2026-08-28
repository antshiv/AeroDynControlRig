# Aircraft Contract

AeroDyn loads the aircraft and controller configuration from two versioned JSON artifacts rather than embedding vehicle constants in C++.

## Artifacts

- `config/reference-quad.aircraft.json` describes frames, mass, centre of mass, the full inertia tensor, gravity, rotor geometry, spin direction, thrust and torque coefficients, and speed limits.
- `config/reference-quad.coefficients.json` identifies the exact aircraft revision and carries controller timing, PID gains, provenance, and acceptance state.
- `schemas/aircraft-spec-v1.schema.json` and `schemas/controller-coefficients-v1.schema.json` define their portable formats.

The coefficient loader rejects a bundle whose `aircraft_id` or `aircraft_revision` does not match the loaded aircraft. A geometry, mass, inertia, motor, propeller, or identified-coefficient change therefore requires a new aircraft revision and a newly validated coefficient bundle.

AeroDyn applies each rotor's speed limit before plant propagation and exposes command saturation in simulation state. Non-finite motor commands reject the step and pause the simulation.

## Coordinate Contract

- Inertial frame: NED (north, east, down).
- Body frame: FRD (forward, right, down).
- Quaternion: scalar-first `[w, x, y, z]`, rotating body vectors into the inertial frame.
- Rotor positions and thrust axes: body-frame metres and unit vectors.

Physical quadrotor thrust points along body `-Z` in this convention. Any controller or firmware mixer that uses positive collective as an abstract control input must perform that sign conversion at its adapter boundary. The physical model must not change signs to accommodate a mixer convention.

## Evidence Flow

```text
FreeCAD geometry / measured mass and inertia
                    |
rotorDynamics -> OpenFOAM -> thrust stand
                    |
          aircraft specification
                    |
system identification -> controller tuning -> HIL
                    |
          coefficient bundle
                    |
       AeroDyn / ASR-FC generator / Gazebo
```

The checked-in reference bundle is `simulation_only` and explicitly rejects physical-flight approval. It contains provisional gains for SIL work, not flight-ready values.

## Build The Contract Test

```bash
cmake -S . -B build_contract \
  -DBUILD_TESTING=ON \
  -DBUILD_AERODYN_GUI=OFF
cmake --build build_contract
ctest --test-dir build_contract --output-on-failure
```

The next adapters should consume these same artifacts. They should not introduce separate copies of mass, inertia, rotor placement, aerodynamic coefficients, timing, or gains.

## Generate CAD And Rendering Geometry

The headless FreeCAD generator consumes the same accepted aircraft specification:

```bash
tools/generate_aircraft_geometry.sh
```

It emits a native FreeCAD document, STEP exchange model, STL analysis/printing mesh, OBJ rendering mesh, and checksum manifest under `assets/generated/`. FRD coordinates remain authoritative; source dimensions are metres and CAD exports are millimetres.

The STL and OBJ exports are byte-reproducible for an unchanged specification and generator. FreeCAD and STEP embed application metadata, so their checksums prove the exact artifact used but are not expected to remain identical after regeneration. The generator removes FreeCAD backup files before publishing its artifact set.

The OpenGL scene loads the generated OBJ at startup. It applies the same rigid-body pose produced by `dynamic_models`; it does not yet animate individual rotors or imply that the provisional controller gains have been accepted for hardware.

See [Simulation Pipeline](simulation-pipeline.md) for the implemented boundary and the next coefficient-fitting work.
