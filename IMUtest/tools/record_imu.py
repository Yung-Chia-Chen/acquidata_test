#!/usr/bin/env python3
import argparse
import csv
import time
from pathlib import Path

import serial
from serial.tools import list_ports


CSV_HEADER = [
    "host_time_s",
    "type",
    "t_ms",
    "ax_g",
    "ay_g",
    "az_g",
    "gx_dps",
    "gy_dps",
    "gz_dps",
    "temp_c",
    "accel_norm_g",
]


def guess_port() -> str:
    ports = list(list_ports.comports())
    preferred = [
        port
        for port in ports
        if "usb" in (port.device or "").lower()
        or "wch" in (port.description or "").lower()
        or "cp210" in (port.description or "").lower()
        or "uart" in (port.description or "").lower()
        or "jtag" in (port.description or "").lower()
    ]
    candidates = preferred or ports
    if not candidates:
        raise SystemExit("No serial ports found. Plug in the ESP32 first.")
    return candidates[0].device


def main() -> int:
    parser = argparse.ArgumentParser(description="Record ESP32 MPU6050 CSV output.")
    parser.add_argument("--port", default=None, help="Serial port, for example /dev/cu.usbmodemXXXX.")
    parser.add_argument("--baud", type=int, default=115200)
    parser.add_argument("--out", default=None, help="CSV output path.")
    args = parser.parse_args()

    port = args.port or guess_port()
    out_path = Path(args.out or f"bike_imu_{time.strftime('%Y%m%d_%H%M%S')}.csv")

    print(f"Recording {port} -> {out_path}")
    print("Stop with Ctrl+C.")

    with serial.Serial(port, args.baud, timeout=1) as ser, out_path.open("w", newline="") as f:
        writer = csv.writer(f)
        writer.writerow(CSV_HEADER)
        f.flush()

        while True:
            line = ser.readline().decode("utf-8", errors="replace").strip()
            if not line:
                continue
            print(line)
            if not line.startswith("IMU,"):
                continue

            parts = line.split(",")
            if len(parts) != len(CSV_HEADER) - 1:
                continue

            writer.writerow([f"{time.time():.3f}", *parts])
            f.flush()


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except KeyboardInterrupt:
        print("\nStopped.")
        raise SystemExit(0)
