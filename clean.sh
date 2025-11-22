#!/bin/bash

# ESP32 Clean Script
# Removes build artifacts and temporary files

set -e

# Get script directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Configuration
BUILD_PATH="$SCRIPT_DIR/build"

echo "=== Cleaning ESP32 Build Artifacts ==="

# Remove build directory
if [ -d "$BUILD_PATH" ]; then
    echo "Removing build directory: $BUILD_PATH"
    rm -rf "$BUILD_PATH"
    echo "Build directory removed"
else
    echo "Build directory does not exist: $BUILD_PATH"
fi

# Remove arduino-cli cache (optional - uncomment if needed)
# if [ -d "$SCRIPT_DIR/.arduino15" ]; then
#     echo "Removing arduino-cli cache: $SCRIPT_DIR/.arduino15"
#     rm -rf "$SCRIPT_DIR/.arduino15"
#     echo "Arduino CLI cache removed"
# fi

# Remove any temporary files
echo "Removing temporary files..."
find "$SCRIPT_DIR" -type f -name "*.tmp" -delete 2>/dev/null || true
find "$SCRIPT_DIR" -type f -name "*.bak" -delete 2>/dev/null || true
find "$SCRIPT_DIR" -type f -name "*~" -delete 2>/dev/null || true

echo ""
echo "=== Clean Complete ==="
echo "All build artifacts have been removed"
echo ""
echo "To rebuild: ./build.sh"
