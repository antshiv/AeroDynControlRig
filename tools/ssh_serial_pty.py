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


REMOTE_BRIDGE = r"""
import errno
import os
import select
import sys
import termios
import tty

device = sys.argv[1]
serial_fd = os.open(device, os.O_RDWR | os.O_NOCTTY | os.O_NONBLOCK)
tty.setraw(serial_fd)
termios.tcflush(serial_fd, termios.TCIOFLUSH)
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
    os.close(serial_fd)
"""


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--host", default="w530", help="SSH host or config alias")
    parser.add_argument("--device", required=True, help="remote serial device")
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


def main() -> int:
    args = parse_args()
    link = Path(args.link)
    master_fd, slave_fd = os.openpty()
    tty.setraw(slave_fd)
    slave_path = os.ttyname(slave_fd)
    install_link(link, slave_path)

    encoded = base64.b64encode(REMOTE_BRIDGE.encode("utf-8")).decode("ascii")
    remote_command = (
        "python3 -u -c \"import base64;"
        f"exec(base64.b64decode('{encoded}'))\" {shlex.quote(args.device)}"
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
