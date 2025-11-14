#!/bin/bash

set -e  # Exit on error

# Detect platform
PLATFORM=$(uname -s)
echo "Detected platform: $PLATFORM"

# Check if make is available
if ! command -v make &> /dev/null; then
    echo "Error: 'make' not found. Please install build tools:"
    if [ "$PLATFORM" = "Darwin" ]; then
        echo "  xcode-select --install"
    elif [ "$PLATFORM" = "Linux" ]; then
        echo "  sudo apt-get install build-essential"
    fi
    exit 1
fi

# Check dependencies
echo "Checking dependencies..."
make check-deps

# Build and install
echo ""
echo "Building quote for $PLATFORM..."
make clean
make

echo ""
echo "Installing quote..."
make install

echo ""
echo "Installation complete! 🎉"
echo ""
echo "Usage examples:"
echo "  quote -s AAPL           # Get Apple stock quote"
echo "  quote -s TSLA -w        # Watch Tesla in real-time"
echo "  quote -s SHOP -e TO     # Toronto Stock Exchange"
echo ""