#!/usr/bin/env python3
import argparse
import glob
import html
import json
import os
import select
import socketserver
import sys
import termios
from http.server import SimpleHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path
from urllib.parse import parse_qs, urlparse


BAUD_RATES = {
    9600: termios.B9600,
    19200: termios.B19200,
    38400: termios.B38400,
    57600: termios.B57600,
    115200: termios.B115200,
    230400: termios.B230400,
    460800: termios.B460800,
    921600: termios.B921600,
}


class SerialDashboardHandler(SimpleHTTPRequestHandler):
    server_version = "CCPASerialDashboard/1.0"

    def do_GET(self):
        parsed = urlparse(self.path)
        if parsed.path in ("/", "/index.html"):
            self.path = "/tools/ccpa_telemetry_dashboard.html"
            return super().do_GET()
        if parsed.path == "/ports":
            return self.list_ports()
        if parsed.path == "/events":
            return self.stream_serial(parsed.query)
        return super().do_GET()

    def log_message(self, fmt, *args):
        sys.stderr.write("[%s] %s\n" % (self.log_date_time_string(), fmt % args))

    def list_ports(self):
        ports = find_serial_ports()
        body = json.dumps({"ports": ports}).encode("utf-8")
        self.send_response(200)
        self.send_header("Content-Type", "application/json; charset=utf-8")
        self.send_header("Cache-Control", "no-cache")
        self.send_header("Content-Length", str(len(body)))
        self.end_headers()
        self.wfile.write(body)

    def stream_serial(self, query):
        params = parse_qs(query)
        serial_port = params.get("port", [""])[0]
        baud_text = params.get("baud", [str(self.server.default_baud_rate)])[0]

        if serial_port not in find_serial_ports():
            self.send_error(400, "Unknown serial port")
            return

        try:
            baud_rate = int(baud_text)
        except ValueError:
            self.send_error(400, "Invalid baud rate")
            return

        self.send_response(200)
        self.send_header("Content-Type", "text/event-stream; charset=utf-8")
        self.send_header("Cache-Control", "no-cache")
        self.send_header("Connection", "keep-alive")
        self.send_header("X-Accel-Buffering", "no")
        self.end_headers()

        try:
            with open_serial(serial_port, baud_rate) as serial_file:
                serial_fd = serial_file.fileno()
                self.write_event(f"Serial opened: {serial_port} @ {baud_rate}")
                buffer = b""
                while True:
                    readable, _, _ = select.select([serial_fd], [], [], 1.0)
                    if not readable:
                        self.wfile.write(b": keepalive\n\n")
                        self.wfile.flush()
                        continue

                    try:
                        chunk = os.read(serial_fd, 1024)
                    except BlockingIOError:
                        continue
                    if not chunk:
                        continue
                    buffer += chunk.replace(b"\r", b"")

                    while b"\n" in buffer:
                        line, buffer = buffer.split(b"\n", 1)
                        text = line.decode("utf-8", errors="replace").strip()
                        if text:
                            self.write_event(text)
        except (BrokenPipeError, ConnectionResetError):
            return
        except Exception as exc:
            try:
                self.write_event(f"Serial error: {exc}")
            except (BrokenPipeError, ConnectionResetError):
                return

    def write_event(self, text):
        escaped = html.escape(text, quote=False)
        for line in escaped.splitlines() or [""]:
            self.wfile.write(f"data: {line}\n".encode("utf-8"))
        self.wfile.write(b"\n")
        self.wfile.flush()


class DashboardServer(ThreadingHTTPServer):
    daemon_threads = True
    allow_reuse_address = True

    def __init__(self, server_address, handler_class, default_baud_rate):
        super().__init__(server_address, handler_class)
        self.default_baud_rate = default_baud_rate


def find_serial_ports():
    patterns = (
        "/dev/ttyUSB*",
        "/dev/ttyACM*",
        "/dev/serial/by-id/*",
        "/dev/serial/by-path/*",
    )
    ports = []
    seen = set()
    for pattern in patterns:
        for candidate in sorted(glob.glob(pattern)):
            path = os.path.realpath(candidate) if candidate.startswith("/dev/serial/") else candidate
            if path in seen:
                continue
            seen.add(path)
            ports.append(candidate)
    return ports


def open_serial(port, baud_rate):
    if baud_rate not in BAUD_RATES:
        raise ValueError(f"unsupported baud rate: {baud_rate}")

    fd = os.open(port, os.O_RDONLY | os.O_NOCTTY | os.O_NONBLOCK)
    try:
        attrs = termios.tcgetattr(fd)
        attrs[0] = 0
        attrs[1] = 0
        attrs[2] = termios.CLOCAL | termios.CREAD | termios.CS8
        attrs[3] = 0
        attrs[4] = BAUD_RATES[baud_rate]
        attrs[5] = BAUD_RATES[baud_rate]
        attrs[6][termios.VMIN] = 0
        attrs[6][termios.VTIME] = 0
        termios.tcsetattr(fd, termios.TCSANOW, attrs)
        return os.fdopen(fd, "rb", buffering=0)
    except Exception:
        os.close(fd)
        raise


def main():
    parser = argparse.ArgumentParser(description="Serve CCPA dashboard and stream Raspberry Pi serial lines.")
    parser.add_argument("--baud", type=int, default=115200, help="serial baud rate")
    parser.add_argument("--host", default="0.0.0.0", help="HTTP bind host")
    parser.add_argument("--port", type=int, default=8000, help="HTTP port")
    args = parser.parse_args()

    project_root = Path(__file__).resolve().parents[1]
    os.chdir(project_root)

    socketserver.TCPServer.allow_reuse_address = True
    server = DashboardServer((args.host, args.port), SerialDashboardHandler, args.baud)
    print(f"Serving dashboard: http://{args.host}:{args.port}")
    print(f"Select serial port from the dashboard. Default baud: {args.baud}")
    server.serve_forever()


if __name__ == "__main__":
    main()
