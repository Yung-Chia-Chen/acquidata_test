#!/usr/bin/env python3
import argparse
import os
import signal
import subprocess
import sys
import time

try:
    import RPi.GPIO as GPIO
except ImportError:
    print("Error: RPi.GPIO module not found. Install with 'sudo apt install python3-rpi.gpio' or 'pip install RPi.GPIO'", file=sys.stderr)
    sys.exit(1)

HERE = os.path.abspath(os.path.dirname(__file__))
SCRIPT_PATH = os.path.join(HERE, "tools", "record_three_esp32.sh")
BUTTON_PIN = 27
LED_PIN = 23
RED_LED_PIN = 23
BLUE_LED_PIN = None
LED_ACTIVE_HIGH = True  # 如果 LED 接到 GPIO 直接點亮，保持 True；如果 LED 接到 3.3V 並接 GPIO 拉低點亮，設 False
LED_PINS = []

process = None
running = False
args = None
last_button_time = 0.0


def parse_args():
    parser = argparse.ArgumentParser(description="GPIO button toggle for three ESP32 data collectors.")
    parser.add_argument("--imu-a", default="/dev/ttyUSB0", help="IMU A serial port")
    parser.add_argument("--imu-b", default="/dev/ttyUSB1", help="IMU B serial port")
    parser.add_argument("--telemetry", default="/dev/ttyACM0", help="Telemetry serial port")
    parser.add_argument("--baud", default="115200", help="Baud rate for all ports")
    parser.add_argument("--outdir", default="csv_logs", help="Base output directory")
    parser.add_argument("--button-pin", type=int, default=BUTTON_PIN, help="BCM GPIO pin for the button")
    parser.add_argument("--led-pin", type=int, default=LED_PIN, help="BCM GPIO pin for the red LED")
    parser.add_argument("--led-red-pin", type=int, default=LED_PIN, help="BCM GPIO pin for the red LED")
    parser.add_argument("--led-blue-pin", type=int, default=None, help="BCM GPIO pin for the blue LED")
    parser.add_argument("--led-active-low", action="store_true", help="Use active-low LED wiring (LED on when GPIO low)")
    parser.add_argument("--test-led", action="store_true", help="Turn the LED on briefly for wiring verification then exit")
    return parser.parse_args()


def setup_gpio():
    GPIO.setmode(GPIO.BCM)
    GPIO.setwarnings(False)

    if not LED_PINS:
        raise RuntimeError("No LED pins configured")

    for pin in LED_PINS:
        GPIO.setup(pin, GPIO.OUT)

    GPIO.setup(BUTTON_PIN, GPIO.IN, pull_up_down=GPIO.PUD_UP)
    set_led_state(False, True)
    print(f"LED pins: {', '.join(str(pin) for pin in LED_PINS)}")
    print(f"LED polarity: {'active-high' if LED_ACTIVE_HIGH else 'active-low'}; initial blue-on state")


def set_led_state(red_on: bool, blue_on: bool):
    red_value = GPIO.HIGH if (red_on == LED_ACTIVE_HIGH) else GPIO.LOW
    blue_value = GPIO.HIGH if (blue_on == LED_ACTIVE_HIGH) else GPIO.LOW
    if RED_LED_PIN is not None:
        GPIO.output(RED_LED_PIN, red_value)
    if BLUE_LED_PIN is not None:
        GPIO.output(BLUE_LED_PIN, blue_value)
    pin_states = []
    if RED_LED_PIN is not None:
        pin_states.append(f"GPIO{RED_LED_PIN} {'HIGH' if red_value == GPIO.HIGH else 'LOW'}")
    if BLUE_LED_PIN is not None:
        pin_states.append(f"GPIO{BLUE_LED_PIN} {'HIGH' if blue_value == GPIO.HIGH else 'LOW'}")
    print(f"set_led_state(red_on={red_on}, blue_on={blue_on}) -> {' '.join(pin_states)}")


def start_collection(args):
    global process, running
    if running and process is not None and process.poll() is None:
        print("Collection already running")
        return

    if not os.path.isfile(SCRIPT_PATH):
        print(f"Error: cannot find script at {SCRIPT_PATH}")
        return

    cmd = [SCRIPT_PATH,
           "--imu-a", args.imu_a,
           "--imu-b", args.imu_b,
           "--telemetry", args.telemetry,
           "--baud", args.baud,
           "--outdir", args.outdir]

    print("Starting data collection:")
    print(" ".join(cmd))
    process = subprocess.Popen(cmd, cwd=HERE, preexec_fn=os.setsid)
    running = True
    set_led_state(True, False)
    print("Red LED on; data collection started")


def stop_collection():
    global process, running
    if process is None:
        print("No collection process to stop")
        running = False
        set_led_state(False, True)
        return

    if process.poll() is not None:
        print("Collection process already stopped")
        running = False
        set_led_state(False, True)
        process = None
        return

    print("Stopping data collection...")
    try:
        os.killpg(os.getpgid(process.pid), signal.SIGINT)
    except Exception as exc:
        print(f"Warning: failed to send SIGINT to process group: {exc}")

    try:
        process.wait(timeout=30)
    except subprocess.TimeoutExpired:
        print("Collection process did not exit in time; killing")
        try:
            os.killpg(os.getpgid(process.pid), signal.SIGTERM)
        except Exception:
            pass
        process.wait(timeout=10)

    running = False
    set_led_state(False, True)
    print("Blue LED on; data collection stopped")


def button_callback(channel):
    global running, args, last_button_time
    now = time.time()
    if now - last_button_time < 0.3:
        print("Button event ignored due to debounce window")
        return

    last_button_time = now
    print("Button press detected")

    if running:
        stop_collection()
    else:
        if args is None:
            print("Error: args not initialized")
            return
        start_collection(args)


def cleanup(signum=None, frame=None):
    print("Cleaning up...")
    if running:
        stop_collection()
    GPIO.cleanup()
    sys.exit(0)


if __name__ == "__main__":
    args = parse_args()
    BUTTON_PIN = args.button_pin
    RED_LED_PIN = args.led_red_pin
    BLUE_LED_PIN = args.led_blue_pin
    LED_ACTIVE_HIGH = not args.led_active_low
    LED_PINS = [pin for pin in (RED_LED_PIN, BLUE_LED_PIN) if pin is not None]
    setup_gpio()
    signal.signal(signal.SIGINT, cleanup)
    signal.signal(signal.SIGTERM, cleanup)

    print("GPIO data control started")
    print(f"Button: GPIO{BUTTON_PIN}, Red LED: GPIO{RED_LED_PIN}, Blue LED: GPIO{BLUE_LED_PIN}")
    print(f"LED polarity: {'active-high' if LED_ACTIVE_HIGH else 'active-low'}")
    print("Press button once to start collection, press again to stop.")

    GPIO.add_event_detect(BUTTON_PIN, GPIO.FALLING, callback=button_callback, bouncetime=300)

    if args.test_led:
        print("Running LED test mode")
        set_led_state(True, False)
        time.sleep(2)
        set_led_state(False, True)
        cleanup()

    try:
        while True:
            if running and process is not None and process.poll() is not None:
                print("Data collection process exited unexpectedly")
                running = False
                set_led_state(False, True)
                process = None
            time.sleep(0.2)
    finally:
        cleanup()
