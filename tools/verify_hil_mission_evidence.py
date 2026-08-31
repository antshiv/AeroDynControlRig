#!/usr/bin/env python3
"""Validate a captured AeroDyn physical HIL mission CSV."""

from __future__ import annotations

import argparse
import csv
import math
from pathlib import Path


STAGES = ["grounded", "takeoff", "joystick_flight", "landing", "grounded_complete"]
MOTORS = [f"motor_{index}_rad_s" for index in range(1, 5)]


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("csv", type=Path)
    parser.add_argument("--runs", type=int, default=3)
    parser.add_argument("--dt", type=float, default=0.01)
    return parser.parse_args()


def require(condition: bool, message: str) -> None:
    if not condition:
        raise ValueError(message)


def main() -> int:
    args = parse_args()
    with args.csv.open(newline="", encoding="utf-8") as handle:
        rows = list(csv.DictReader(handle))
    require(bool(rows), "evidence CSV has no rows")

    run_ids = sorted({int(row["run"]) for row in rows})
    require(run_ids == list(range(1, args.runs + 1)), "unexpected run IDs")
    sessions: set[int] = set()

    for run_id in run_ids:
        run = [row for row in rows if int(row["run"]) == run_id]
        sequences = [int(row["sequence"]) for row in run]
        require(sequences == list(range(1, len(run) + 1)),
                f"run {run_id} has a missing or repeated sequence")
        session_ids = {int(row["session_id"]) for row in run}
        require(len(session_ids) == 1, f"run {run_id} changed session identity")
        session_id = session_ids.pop()
        require(session_id not in sessions, "session identity was reused")
        sessions.add(session_id)

        observed_stages = list(dict.fromkeys(row["stage"] for row in run))
        require(observed_stages == STAGES,
                f"run {run_id} stage order is {observed_stages}")
        require(all(int(row["fault_flags"]) == 0 for row in run),
                f"run {run_id} contains a controller fault")
        require(all(math.isclose(float(row["sim_dt_s"]), args.dt,
                                 rel_tol=0.0, abs_tol=1e-9) for row in run),
                f"run {run_id} changed simulated dt")
        require(all(int(row["mcu_execution_us"]) > 0 for row in run),
                f"run {run_id} contains no MCU execution evidence")

        for boundary in (run[0], run[-1]):
            require(boundary["phase"] == "grounded",
                    f"run {run_id} does not start and end grounded")
            require(all(float(boundary[motor]) == 0.0 for motor in MOTORS),
                    f"run {run_id} grounded boundary has motor authority")
        require(any(float(row[motor]) > 0.0 for row in run for motor in MOTORS),
                f"run {run_id} never produced a virtual motor command")

    print(f"physical HIL evidence passed: {len(rows)} rows, {args.runs} runs")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (KeyError, OSError, ValueError) as error:
        print(f"physical HIL evidence failed: {error}")
        raise SystemExit(1)
