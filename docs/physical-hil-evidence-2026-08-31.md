# Physical nRF5340 HIL evidence

On 2026-08-31, AeroDyn completed three deterministic, motor-disabled HIL
missions through a physical nRF5340 DK connected to the W530. AeroDyn ran on
the T14, reached the serial device through authenticated SSH, propagated the
6-DOF plant with RK4, and accepted motor commands only from the checked ASR-FC
endpoint running on the MCU.

The tested mission was:

1. Grounded with all four virtual motors at zero.
2. Takeoff to the declared 1.5 m hover altitude.
3. Settled hover.
4. Joystick-derived translation and yaw.
5. Controlled landing.
6. Grounded with all four virtual motors at zero.

Each run used a fresh session identity and completed 992 checked MCU exchanges
and 3,968 RK4 substeps. All three runs followed the same stage counts and
finished at the same simulated position and attitude within the CSV precision.
No response carried a fault flag.

| Evidence | Result |
|---|---:|
| Runs | 3/3 passed |
| Simulated control period | 10 ms |
| MCU execution, median | 890 us |
| MCU execution, p95 | 894 us |
| MCU execution, maximum | 1,474 us |
| SSH serial round trip, median range | 23.338-23.398 ms |
| SSH serial round trip, maximum | 25.243 ms |
| Maximum altitude | 1.525103 m |
| Maximum virtual motor speed | 648.266418 rad/s |
| Faulted rows | 0/2,976 |

The remote SSH round trip is longer than the 10 ms simulated period. This is a
deterministic lockstep HIL result, not a claim that the T14-to-W530 network path
runs the controller in real time. The physical nRF computation itself remains
below 1.5 ms in this capture. A direct local serial or lower-latency transport
is required before claiming a real-time 100 Hz hardware loop.

## Contract defects found

The physical gate exposed two defects that a disarmed one-frame probe could not:

- AeroDyn used signed FRD body-axis thrust while the HIL wire contract requires
  a non-negative collective magnitude.
- The nRF endpoint used `+Z` rotor axes and a different rotor order while the
  declared AeroDyn aircraft uses FRD `-Z` thrust axes.

[ASR-FC PR #2](https://github.com/antshiv/ASR-FC/pull/2) maps the wire
magnitude onto the common configured rotor-axis sign and aligns the nRF
endpoint with the reference aircraft. AeroDyn converts its signed desktop
collective to a magnitude only at the wire boundary.

## Fault gates

| Fault | Automated evidence | Required host result |
|---|---|---|
| CRC-corrupt response | `aerodyn_nrf_hil_transport_test` | Reject response |
| Stale/wrong identity | transport test and ASR-FC endpoint test | Reject response |
| Delayed response | transport test | Timeout |
| Serial disconnect | `aerodyn_nrf_hil_fail_closed_test` | Pause plant, disable pilot, zero four motors |
| Duplicate sensor frame | ASR-FC endpoint test | Enter checked failsafe |
| New session | Three physical runs with distinct session IDs | Reset endpoint state and restart sequence at one |

The SSH bridge now takes an exclusive lock on the remote serial device and uses
a unique remote lease. Shutdown explicitly terminates that lease, preventing
orphan readers from consuming responses from a later HIL run.

## Reproduce

```bash
python3 tools/ssh_serial_pty.py \
  --host <ssh-alias> \
  --device <remote-/dev/serial/by-id-path> \
  --baud 115200 \
  --link /tmp/asr-fc-hil

./build/aerodyn_nrf_hil_probe /tmp/asr-fc-hil
./build/aerodyn_nrf_hil_mission \
  /tmp/asr-fc-hil artifacts/hil/physical-mission-3x.csv 3
```

The complete capture is `artifacts/hil/physical-mission-3x.csv` (SHA-256
`f6545192f6804f55b8ebb019942923efe41731364496e5388c39274190ccbb28`).

## Boundary

This is still simulated-sensor HIL with virtual motor outputs. It does not yet
include live CEVA FSM300 attitude, barometer/ToF aiding, wind, noisy sensors,
actuator lag, motor power, or physical PWM. The firmware contains a compile-time
assertion that physical outputs are disabled, and the coefficient bundle remains
marked `simulation_only`.
