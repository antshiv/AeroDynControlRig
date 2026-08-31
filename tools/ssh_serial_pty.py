#!/usr/bin/env python3
"""Expose a remote serial device as a local PTY through authenticated SSH."""

from __future__ import annotations

import argparse
import base64
import errno
import os
from pathlib import Path
import select
import shlex
import signal
import subprocess
import sys
import termios
import tty
import uuid


REMOTE_BRIDGE = r"""
import errno
import fcntl
import os
import select
import sys
import termios
import tty

device = sys.argv[1]
baud = int(sys.argv[2])
lease = sys.argv[3]
lease_fd = os.open(lease, os.O_WRONLY | os.O_CREAT | os.O_EXCL, 0o600)
os.write(lease_fd, str(os.getpid()).encode("ascii"))
os.close(lease_fd)
serial_fd = -1
stdin_fd = sys.stdin.fileno()
stdout_fd = sys.stdout.fileno()

def write_all(fd, data):
    offset = 0
    while offset < len(data):
        try:
            offset += os.write(fd, data[offset:])
        except BlockingIOError:
            select.select([], [fd], [])

try:
    serial_fd = os.open(device, os.O_RDWR | os.O_NOCTTY | os.O_NONBLOCK)
    fcntl.flock(serial_fd, fcntl.LOCK_EX | fcntl.LOCK_NB)
    tty.setraw(serial_fd)
    serial_attrs = termios.tcgetattr(serial_fd)
    baud_constant = getattr(termios, f"B{baud}")
    serial_attrs[4] = baud_constant
    serial_attrs[5] = baud_constant
    termios.tcsetattr(serial_fd, termios.TCSANOW, serial_attrs)
    termios.tcflush(serial_fd, termios.TCIOFLUSH)
    while True:
        readable, _, _ = select.select([serial_fd, stdin_fd], [], [])
        if serial_fd in readable:
            try:
                data = os.read(serial_fd, 4096)
            except BlockingIOError:
                data = b""
            if data:
                write_all(stdout_fd, data)
        if stdin_fd in readable:
            data = os.read(stdin_fd, 4096)
            if not data:
                break
            write_all(serial_fd, data)
finally:
    if serial_fd >= 0:
        os.close(serial_fd)
    try:
        os.unlink(lease)
    except FileNotFoundError:
        pass
"""

REMOTE_STOP = r"""
import os
import signal
import sys

lease = sys.argv[1]
try:
    with open(lease, "r", encoding="ascii") as handle:
        pid = int(handle.read())
    with open(f"/proc/{pid}/cmdline", "rb") as handle:
        command = handle.read()
    if lease.encode("utf-8") in command:
        os.kill(pid, signal.SIGTERM)
except (FileNotFoundError, ProcessLookupError, ValueError):
    pass
finally:
    try:
        os.unlink(lease)
    except FileNotFoundError:
        pass
"""


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--host", required=True, help="SSH host or config alias")
    parser.add_argument("--device", required=True, help="remote serial device")
    parser.add_argument(
        "--baud", type=int, default=115200, help="remote serial baud rate"
    )
    parser.add_argument(
        "--link", default="/tmp/asr-fc-hil", help="local PTY symlink to create"
    )
    return parser.parse_args()


def install_link(link: Path, target: str) -> None:
    if link.exists() and not link.is_symlink():
        raise RuntimeError(f"refusing to replace non-symlink path: {link}")
    if link.is_symlink():
        link.unlink()
    link.symlink_to(target)


def write_all(fd: int, data: bytes) -> None:
    offset = 0
    while offset < len(data):
        try:
            offset += os.write(fd, data[offset:])
        except BlockingIOError:
            select.select([], [fd], [])


def stop_remote_bridge(host: str, lease: str) -> None:
    encoded = base64.b64encode(REMOTE_STOP.encode("utf-8")).decode("ascii")
    command = (
        "python3 -c \"import base64;"
        f"exec(base64.b64decode('{encoded}'))\" {shlex.quote(lease)}"
    )
    try:
        subprocess.run(
            ["ssh", "-T", host, command],
            stdin=subprocess.DEVNULL,
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
            timeout=5,
            check=False,
        )
    except subprocess.TimeoutExpired:
        pass


def main() -> int:
    args = parse_args()
    link = Path(args.link)
    master_fd, slave_fd = os.openpty()
    tty.setraw(slave_fd)
    slave_path = os.ttyname(slave_fd)
    install_link(link, slave_path)
    lease = f"/tmp/asr-fc-serial-bridge-{uuid.uuid4().hex}.pid"

    encoded = base64.b64encode(REMOTE_BRIDGE.encode("utf-8")).decode("ascii")
    remote_command = (
        "python3 -u -c \"import base64;"
        f"exec(base64.b64decode('{encoded}'))\" {shlex.quote(args.device)} "
        f"{args.baud} {shlex.quote(lease)}"
    )
    process = subprocess.Popen(
        ["ssh", "-T", args.host, remote_command],
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        bufsize=0,
    )
    assert process.stdin is not None
    assert process.stdout is not None
    ssh_input = process.stdin.fileno()
    ssh_output = process.stdout.fileno()

    def stop(_signum: int, _frame: object) -> None:
        process.terminate()

    signal.signal(signal.SIGINT, stop)
    signal.signal(signal.SIGTERM, stop)
    print(f"{link} -> {slave_path} -> {args.host}:{args.device}", flush=True)

    try:
        while process.poll() is None:
            readable, _, _ = select.select([master_fd, ssh_output], [], [], 0.5)
            if master_fd in readable:
                try:
                    data = os.read(master_fd, 4096)
                except OSError as error:
                    if error.errno == errno.EIO:
                        continue
                    raise
                if data:
                    write_all(ssh_input, data)
            if ssh_output in readable:
                data = os.read(ssh_output, 4096)
                if not data:
                    break
                write_all(master_fd, data)
    finally:
        stop_remote_bridge(args.host, lease)
        process.terminate()
        try:
            process.wait(timeout=3)
        except subprocess.TimeoutExpired:
            process.kill()
            process.wait()
        os.close(master_fd)
        os.close(slave_fd)
        if link.is_symlink() and os.path.realpath(link) == slave_path:
            link.unlink()
    return process.returncode or 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (OSError, RuntimeError) as error:
        print(f"ssh serial bridge failed: {error}", file=sys.stderr)
        raise SystemExit(1)
