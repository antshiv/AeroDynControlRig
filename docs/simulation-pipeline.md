# Simulation Pipeline

The current vertical slice uses one versioned aircraft specification for both the rigid-body plant and visible geometry:

```text
aircraft specification
    |-- mass, inertia, rotor positions, CT/CQ --> dynamic_models --> pose
    |-- body, arms, mounts, landing gear ------> FreeCAD
                                                       |
                                             FCStd / STEP / STL / OBJ
                                                       |
pose ------------------------------------------------> OpenGL dashboard
```

This removes the previous failure mode where the simulator could display one vehicle while propagating another vehicle's mass, inertia, and rotor layout. Contract tests reject invalid frames, geometry, revisions, and physical values. A mesh test checks that the generated OBJ remains loadable and has metre-scale bounds after conversion from CAD millimetres.

## What Is Connected

- Aircraft identity, revision, frame conventions, geometry, mass, inertia, gravity, rotor placement, thrust and torque coefficients, and speed limits.
- Transactional RK4 plant propagation from `dynamic_models`.
- Generated FreeCAD, STEP, STL, and OBJ artifacts.
- Generated OBJ rendering under the plant's model transform.
- A controller-coefficient bundle linked to the exact aircraft revision and marked `simulation_only`.
- Logitech F310 or Dual Action Mode 2 pilot commands through `controlSystems`' quaternion PID and geometry-derived mixer.
- A 40 ms controller cadence feeding the finer checked RK4 plant substeps.

## What Is Not Yet Connected

- Identified and accepted PID/LQR tuning; current PID gains remain provisional SIL fixtures.
- Per-rotor blade geometry or visible rotor animation.
- OpenFOAM coefficient extraction and mesh-convergence evidence.
- Thrust-stand measurements and system-identification fitting.
- Firmware generation for ASR-FC.
- Gazebo scenarios, environmental models, or ruggedness claims.

The joystick path is documented in [Mode 2 Flight Controls](joystick-controls.md).

## Next Evidence Loop

```text
shared blade specification
    |-- BEMT prediction
    |-- FreeCAD blade surface --> OpenFOAM coarse/medium/fine meshes
    |-- printed propeller ------> thrust stand
                                      |
                         fitted CT/CQ with uncertainty
                                      |
                         new aircraft revision
                                      |
              controller tuning --> SIL --> HIL --> flight evidence
```

OpenFOAM and measured thrust should improve the coefficients used by BEMT and the rigid-body plant rather than becoming disconnected demonstrations. A coefficient revision is accepted only when its provenance, operating range, residuals, and validation status are recorded in the bundle.
