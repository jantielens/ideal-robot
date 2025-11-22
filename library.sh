#!/bin/bash

# Arduino Library Management Script
# Helps add, remove, and list libraries for the ESP32 project

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
LIBRARIES_FILE="$SCRIPT_DIR/arduino-libraries.txt"
ARDUINO_CLI="$SCRIPT_DIR/bin/arduino-cli"

# Colors for output
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m' # No Color

show_usage() {
    cat << EOF
Arduino Library Management for ESP32 Template

Usage:
  ./library.sh search <keyword>    Search for libraries
  ./library.sh add <library>       Add library to arduino-libraries.txt and install it
  ./library.sh remove <library>    Remove library from arduino-libraries.txt
  ./library.sh list                List configured libraries
  ./library.sh install             Install all libraries from arduino-libraries.txt
  ./library.sh installed           Show currently installed libraries

Examples:
  ./library.sh search mqtt
  ./library.sh add PubSubClient@2.8
  ./library.sh add "Adafruit GFX Library"
  ./library.sh remove ArduinoJson
  ./library.sh list

EOF
}

check_arduino_cli() {
    if [ ! -f "$ARDUINO_CLI" ]; then
        echo -e "${RED}Error: arduino-cli not found at $ARDUINO_CLI${NC}"
        echo "Please run ./setup.sh first"
        exit 1
    fi
}

search_library() {
    local keyword="$1"
    if [ -z "$keyword" ]; then
        echo -e "${RED}Error: Please provide a search keyword${NC}"
        exit 1
    fi
    
    check_arduino_cli
    echo "Searching for libraries matching '$keyword'..."
    "$ARDUINO_CLI" lib search "$keyword"
}

add_library() {
    local library="$1"
    if [ -z "$library" ]; then
        echo -e "${RED}Error: Please provide a library name${NC}"
        exit 1
    fi
    
    # Check if library is already in the file
    if grep -Fxq "$library" "$LIBRARIES_FILE" 2>/dev/null; then
        echo -e "${YELLOW}Library '$library' is already in $LIBRARIES_FILE${NC}"
        return
    fi
    
    # Add library to file
    echo "$library" >> "$LIBRARIES_FILE"
    echo -e "${GREEN}Added '$library' to $LIBRARIES_FILE${NC}"
    
    # Install the library
    check_arduino_cli
    echo "Installing library..."
    "$ARDUINO_CLI" lib install "$library"
    echo -e "${GREEN}Library installed successfully${NC}"
}

remove_library() {
    local library="$1"
    if [ -z "$library" ]; then
        echo -e "${RED}Error: Please provide a library name${NC}"
        exit 1
    fi
    
    if [ ! -f "$LIBRARIES_FILE" ]; then
        echo -e "${RED}Error: $LIBRARIES_FILE not found${NC}"
        exit 1
    fi
    
    # Remove library from file
    if grep -Fxq "$library" "$LIBRARIES_FILE"; then
        # Create temp file without the library
        grep -Fxv "$library" "$LIBRARIES_FILE" > "$LIBRARIES_FILE.tmp"
        mv "$LIBRARIES_FILE.tmp" "$LIBRARIES_FILE"
        echo -e "${GREEN}Removed '$library' from $LIBRARIES_FILE${NC}"
        echo -e "${YELLOW}Note: Library is not uninstalled, only removed from config${NC}"
    else
        echo -e "${YELLOW}Library '$library' not found in $LIBRARIES_FILE${NC}"
    fi
}

list_libraries() {
    if [ ! -f "$LIBRARIES_FILE" ]; then
        echo -e "${YELLOW}No libraries configured (${LIBRARIES_FILE} not found)${NC}"
        return
    fi
    
    echo "Configured libraries in $LIBRARIES_FILE:"
    echo ""
    
    local count=0
    while IFS= read -r line || [ -n "$line" ]; do
        # Skip empty lines and comments
        if [[ -z "$line" || "$line" =~ ^[[:space:]]*# ]]; then
            continue
        fi
        
        library=$(echo "$line" | xargs)
        if [ -n "$library" ]; then
            echo "  - $library"
            count=$((count + 1))
        fi
    done < "$LIBRARIES_FILE"
    
    if [ $count -eq 0 ]; then
        echo "  (no libraries configured yet)"
    fi
    
    echo ""
    echo "Total: $count libraries"
}

install_all() {
    if [ ! -f "$LIBRARIES_FILE" ]; then
        echo -e "${RED}Error: $LIBRARIES_FILE not found${NC}"
        exit 1
    fi
    
    check_arduino_cli
    echo "Installing all libraries from $LIBRARIES_FILE..."
    echo ""
    
    while IFS= read -r line || [ -n "$line" ]; do
        # Skip empty lines and comments
        if [[ -z "$line" || "$line" =~ ^[[:space:]]*# ]]; then
            continue
        fi
        
        library=$(echo "$line" | xargs)
        if [ -n "$library" ]; then
            echo "Installing: $library"
            "$ARDUINO_CLI" lib install "$library" || echo -e "${YELLOW}Warning: Failed to install $library${NC}"
        fi
    done < "$LIBRARIES_FILE"
    
    echo ""
    echo -e "${GREEN}Installation complete${NC}"
}

show_installed() {
    check_arduino_cli
    echo "Currently installed libraries:"
    "$ARDUINO_CLI" lib list
}

# Main command dispatcher
case "${1:-}" in
    search)
        search_library "$2"
        ;;
    add)
        add_library "$2"
        ;;
    remove)
        remove_library "$2"
        ;;
    list)
        list_libraries
        ;;
    install)
        install_all
        ;;
    installed)
        show_installed
        ;;
    *)
        show_usage
        exit 1
        ;;
esac
