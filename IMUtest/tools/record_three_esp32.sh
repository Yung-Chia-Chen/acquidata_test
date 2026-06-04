#!/usr/bin/env bash
set -euo pipefail

usage() {
  cat <<'EOF'
Usage:
  ./tools/record_three_esp32.sh --imu-a /dev/ttyUSB0 --imu-b /dev/ttyUSB1 --telemetry /dev/ttyACM0 [options]

Options:
  --imu-a PORT        Serial port for IMU logger A. Required.
  --imu-b PORT        Serial port for IMU logger B. Required.
  --telemetry PORT    Serial port for sdkTest CAN/CCPA telemetry logger. Required.
  --baud BAUD         Baud rate for all three ports. Default: 115200.
  --outdir DIR        Base output directory. Default: csv_logs.
  --sdk-root DIR      sdkTest project root. Default: /home/chc/code/sdkTest.
  -h, --help          Show this help.

Example:
  ./tools/record_three_esp32.sh --imu-a /dev/ttyUSB0 --imu-b /dev/ttyUSB1 --telemetry /dev/ttyACM0
EOF
}

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

IMU_A_PORT=""
IMU_B_PORT=""
TELEMETRY_PORT=""
BAUD="115200"
OUTDIR="csv_logs"
SDK_ROOT="/home/chc/code/sdkTest"


require_value() {
  local option="$1"
  local value="${2:-}"

  if [[ -z "$value" || "$value" == --* ]]; then
    echo "Error: $option requires a value." >&2
    usage >&2
    exit 1
  fi
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    --imu-a)
      require_value "$1" "${2:-}"
      IMU_A_PORT="${2:-}"
      shift 2
      ;;
    --imu-b)
      require_value "$1" "${2:-}"
      IMU_B_PORT="${2:-}"
      shift 2
      ;;
    --telemetry)
      require_value "$1" "${2:-}"
      TELEMETRY_PORT="${2:-}"
      shift 2
      ;;
    --baud)
      require_value "$1" "${2:-}"
      BAUD="${2:-}"
      shift 2
      ;;
    --outdir)
      require_value "$1" "${2:-}"
      OUTDIR="${2:-}"
      shift 2
      ;;
    --sdk-root)
      require_value "$1" "${2:-}"
      SDK_ROOT="${2:-}"
      shift 2
      ;;
    -h|--help)
      usage
      exit 0
      ;;
    *)
      echo "Unknown argument: $1" >&2
      usage >&2
      exit 1
      ;;
  esac
done

if [[ -z "$IMU_A_PORT" || -z "$IMU_B_PORT" || -z "$TELEMETRY_PORT" ]]; then
  echo "Error: --imu-a, --imu-b, and --telemetry are required." >&2
  usage >&2
  exit 1
fi

if [[ -z "$BAUD" || -z "$OUTDIR" || -z "$SDK_ROOT" ]]; then
  echo "Error: --baud, --outdir, and --sdk-root cannot be empty." >&2
  usage >&2
  exit 1
fi

IMU_SRC="$ROOT/tools/serial_csv_logger.cpp"
IMU_BIN="$ROOT/tools/serial_csv_logger"
TELEMETRY_SRC="$SDK_ROOT/tools/serial_csv_logger.cpp"
TELEMETRY_BIN="$SDK_ROOT/tools/serial_csv_logger"

if [[ ! -f "$IMU_SRC" ]]; then
  echo "Error: IMU logger source not found: $IMU_SRC" >&2
  exit 1
fi

if [[ ! -f "$TELEMETRY_SRC" ]]; then
  echo "Error: telemetry logger source not found: $TELEMETRY_SRC" >&2
  exit 1
fi

echo "Building IMU serial_csv_logger..."
g++ -std=c++17 "$IMU_SRC" -o "$IMU_BIN"

echo "Building sdkTest telemetry serial_csv_logger..."
g++ -std=c++17 "$TELEMETRY_SRC" -o "$TELEMETRY_BIN"

RUN_ID="$(date '+run_%Y%m%d_%H%M%S')"
RUN_DIR="$ROOT/$OUTDIR/$RUN_ID"
IMU_A_DIR="$RUN_DIR/imu_a"
IMU_B_DIR="$RUN_DIR/imu_b"
TELEMETRY_DIR="$RUN_DIR/telemetry"
MANIFEST="$RUN_DIR/manifest.txt"

mkdir -p "$IMU_A_DIR" "$IMU_B_DIR" "$TELEMETRY_DIR"

cat > "$MANIFEST" <<EOF
run_id=$RUN_ID
started_at=$(date '+%Y-%m-%d %H:%M:%S %z')
baud=$BAUD
imu_a_port=$IMU_A_PORT
imu_a_output_dir=$IMU_A_DIR
imu_b_port=$IMU_B_PORT
imu_b_output_dir=$IMU_B_DIR
telemetry_port=$TELEMETRY_PORT
telemetry_output_dir=$TELEMETRY_DIR
imu_logger=$IMU_BIN
telemetry_logger=$TELEMETRY_BIN
sdk_root=$SDK_ROOT
EOF

pids=()

cleanup() {
  local status=$?
  trap - EXIT INT TERM

  if [[ ${#pids[@]} -gt 0 ]]; then
    echo
    echo "Stopping loggers..."
    for pid in "${pids[@]}"; do
      if kill -0 "$pid" 2>/dev/null; then
        kill -INT "$pid" 2>/dev/null || true
      fi
    done
    wait "${pids[@]}" 2>/dev/null || true
  fi

  {
    echo "stopped_at=$(date '+%Y-%m-%d %H:%M:%S %z')"
    echo "exit_status=$status"
  } >> "$MANIFEST"

  echo "Run saved under: $RUN_DIR"
  exit "$status"
}

trap cleanup EXIT INT TERM

echo
echo "Starting three ESP32 CSV loggers at $BAUD baud..."
echo "Run directory: $RUN_DIR"
echo "IMU A:     $IMU_A_PORT -> $IMU_A_DIR"
echo "IMU B:     $IMU_B_PORT -> $IMU_B_DIR"
echo "Telemetry: $TELEMETRY_PORT -> $TELEMETRY_DIR"
echo "Manifest:  $MANIFEST"
echo "Press Ctrl+C to stop."
echo

"$IMU_BIN" "$IMU_A_PORT" "$BAUD" "$IMU_A_DIR" &
pids+=("$!")

"$IMU_BIN" "$IMU_B_PORT" "$BAUD" "$IMU_B_DIR" &
pids+=("$!")

"$TELEMETRY_BIN" "$TELEMETRY_PORT" "$BAUD" "$TELEMETRY_DIR" &
pids+=("$!")

wait "${pids[@]}"
