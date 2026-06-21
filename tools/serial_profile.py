#!/usr/bin/env python3
"""Host wall-clock profiler for @PROFILE name BEGIN/END markers from CHIP firmware."""

import argparse
import re
import sys
import time
from collections import defaultdict
from datetime import datetime

import serial
from serial.tools import list_ports

BAUD = 115200
PROFILE_RE = re.compile(r"^@PROFILE (\S+) (BEGIN|END)\s*$")


def find_board_port():
    ports = [p for p in list_ports.comports() if "F411CE" in p.description]
    if not ports:
        return None
    return ports[0].device


def open_serial(port_arg):
    if port_arg:
        port = port_arg
    else:
        port = find_board_port()
        if not port:
            print("ERROR: Board not found (no port with 'F411CE' in description).")
            print("       Use --port COMx to specify the port manually.")
            sys.exit(1)
        print(f"Found board: {port}")

    try:
        ser = serial.Serial(port, BAUD, timeout=0.5)
    except serial.SerialException as exc:
        print(f"ERROR: Cannot open {port}: {exc}")
        sys.exit(1)

    print(f"Connected to {port} @ {BAUD} — ready")
    return ser


def format_host_time():
    return datetime.now().strftime("%H:%M:%S.") + f"{datetime.now().microsecond // 1000:03d}"


def report_pending(pending):
    open_traces = {name: len(stack) for name, stack in pending.items() if stack}
    if not open_traces:
        return
    print("WARNING: unclosed traces:")
    for name, count in sorted(open_traces.items()):
        print(f"  {name}: {count} unmatched BEGIN(s)")


def handle_profile_line(line, pending, filter_profile):
    match = PROFILE_RE.match(line)
    if not match:
        return False

    name, phase = match.group(1), match.group(2)
    now = time.perf_counter()

    if phase == "BEGIN":
        pending[name].append(now)
        if filter_profile:
            print(f"[{format_host_time()}] @PROFILE {name} BEGIN")
    else:
        if not pending[name]:
            print(f"WARNING: END without BEGIN for '{name}'")
            return True

        t_begin = pending[name].pop()
        elapsed_ms = (now - t_begin) * 1000.0
        print(f"{name}: {elapsed_ms:.1f} ms")

    return True


def main():
    parser = argparse.ArgumentParser(
        description="Profile CHIP firmware blocks via @PROFILE Serial markers (host wall-clock)."
    )
    parser.add_argument("--port", help="Serial port (e.g. COM7). Auto-detect if omitted.")
    parser.add_argument(
        "--filter-profile",
        action="store_true",
        help="Show only @PROFILE lines and timing results.",
    )
    args = parser.parse_args()

    ser = open_serial(args.port)
    pending = defaultdict(list)

    try:
        while True:
            raw = ser.readline()
            if not raw:
                continue

            try:
                line = raw.decode("utf-8", errors="replace").rstrip("\r\n")
            except UnicodeDecodeError:
                continue

            if handle_profile_line(line, pending, args.filter_profile):
                continue

            if not args.filter_profile:
                print(f"[{format_host_time()}] {line}")

    except KeyboardInterrupt:
        print()
        report_pending(pending)
    finally:
        ser.close()


if __name__ == "__main__":
    main()
