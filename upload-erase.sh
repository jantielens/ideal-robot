#!/bin/bash

# ESP32 Flash Erase Script
# Completely erases the ESP32 flash memory

set -e

# Get script directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Configuration
FQBN="esp32:esp32:esp32"
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
        echo "Please specify port manually: $0 <port>"
        exit 1
    fi
else
    PORT="$1"
fi

echo "=== ESP32 Flash Erase Utility ==="
echo ""
echo "WARNING: This will completely erase all data from the ESP32 flash memory!"
echo "Port: $PORT"
echo "Board: $FQBN"
echo ""
read -p "Are you sure you want to continue? (y/N) " -n 1 -r
echo ""

if [[ ! $REPLY =~ ^[Yy]$ ]]; then
    echo "Flash erase cancelled"
    exit 0
fi

# Check if arduino-cli exists in local bin directory
if [ ! -f "$ARDUINO_CLI" ]; then
    echo "Error: arduino-cli is not installed at $ARDUINO_CLI"
    echo "Please run setup.sh first"
    exit 1
fi

echo ""
echo "Erasing flash memory..."
echo ""

# Use esptool.py which is installed with ESP32 platform
# The arduino-cli includes esptool.py with the ESP32 core
ESPTOOL="$HOME/.arduino15/packages/esp32/tools/esptool_py/*/esptool.py"

# Find esptool.py
ESPTOOL_PATH=$(find "$HOME/.arduino15/packages/esp32/tools/esptool_py" -name "esptool.py" -type f 2>/dev/null | head -n 1)

if [ -z "$ESPTOOL_PATH" ]; then
    echo "Error: esptool.py not found"
    echo "Please ensure ESP32 platform is installed (run setup.sh)"
    exit 1
fi

# Erase flash
python3 "$ESPTOOL_PATH" --chip esp32 --port "$PORT" erase_flash

echo ""
echo "=== Flash Erase Complete ==="
echo "The ESP32 flash memory has been completely erased"
echo ""
echo "Next steps:"
echo "  1. Upload new firmware: ./upload.sh"
echo "  2. Or build and upload: ./build.sh && ./upload.sh"
echo ""
echo "Usage: $0 [port]"
echo "Example: $0 /dev/ttyUSB0"
