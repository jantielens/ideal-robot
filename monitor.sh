#!/bin/bash

# ESP32 Serial Monitor Script
# Displays serial output from the ESP32 device

set -e

# Get script directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Configuration
BAUD="${2:-115200}"         # Default baud rate, can be overridden by second argument
ARDUINO_CLI="$SCRIPT_DIR/bin/arduino-cli"

# Auto-detect port if not specified
if [ -z "$1" ]; then
    if [ -e "/dev/ttyUSB0" ]; then
        PORT="/dev/ttyUSB0"
        echo "Auto-detected port: $PORT"
    elif [ -e "/dev/ttyACM0" ]; then
        PORT="/dev/ttyACM0"
        echo "Auto-detected port: $PORT"
    else
        echo "Error: No serial device found at /dev/ttyUSB0 or /dev/ttyACM0"
        echo "Please specify port manually: $0 <port> [baud_rate]"
        exit 1
    fi
else
    PORT="$1"
fi

echo "=== ESP32 Serial Monitor ==="

# Check if arduino-cli exists in local bin directory
if [ ! -f "$ARDUINO_CLI" ]; then
    echo "Error: arduino-cli is not installed at $ARDUINO_CLI"
    echo "Please run setup.sh first"
    exit 1
fi

# Display connection info
echo "Connecting to: $PORT"
echo "Baud rate: $BAUD"
echo ""
echo "Press Ctrl+C to exit"
echo "----------------------------------------"
echo ""

# Start serial monitor
"$ARDUINO_CLI" monitor --port "$PORT" --config baudrate=$BAUD

echo ""
echo "Usage: $0 [port] [baud_rate]"
echo "Example: $0 /dev/ttyUSB0 115200"
