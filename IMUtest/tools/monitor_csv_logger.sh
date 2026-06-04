#!/usr/bin/env bash
set -euo pipefail

PORT="${1:-/dev/ttyUSB0}"
BAUD="${2:-115200}"
OUTDIR="${3:-csv_logs}"

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
SRC="$ROOT/tools/serial_csv_logger.cpp"
BIN="$ROOT/tools/serial_csv_logger"

cd "$ROOT"

echo "Building serial_csv_logger..."
g++ -std=c++17 "$SRC" -o "$BIN"

echo
echo "Starting IMU CSV monitor on $PORT at $BAUD baud..."
echo "CSV files will be saved under: $ROOT/$OUTDIR"
echo "Press Ctrl+C to stop."
echo

"$BIN" "$PORT" "$BAUD" "$OUTDIR"