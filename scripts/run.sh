#!/bin/bash
# GladOS Run Script (for WSL/Linux)
# Runs GladOS in QEMU

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"

cd "$PROJECT_DIR"

DISK_IMAGE="build/disk.img"

if [ ! -f "$DISK_IMAGE" ]; then
    echo "ERROR: Disk image not found at $DISK_IMAGE"
    echo ""
    echo "Please build the project first:"
    echo "  ./scripts/build.sh"
    exit 1
fi

echo "Starting GladOS in QEMU..."
echo ""
echo "QEMU Options:"
echo "  - Memory: 512MB"
echo "  - CPU: 1 core"
echo "  - Display: GTK (or SDL if available)"
echo ""
echo "Press Ctrl+C to exit"
echo ""

# Check if qemu-system-x86_64 is available
if ! command -v qemu-system-x86_64 >/dev/null 2>&1; then
    echo "ERROR: qemu-system-x86_64 not found"
    echo ""
    echo "Please install QEMU:"
    echo "  sudo apt-get install qemu-system-x86"
    exit 1
fi

qemu-system-x86_64 \
    -drive format=raw,file="$DISK_IMAGE" \
    -m 512M \
    -smp 1 \
    -serial stdio \
    "$@"
