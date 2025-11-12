#!/bin/bash
# GladOS Build Script
# Builds the entire OS and creates a bootable disk image

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"

cd "$PROJECT_DIR"

echo "=========================================="
echo "Building GladOS"
echo "=========================================="
echo ""

# Check for required tools
echo "Checking for required tools..."
MISSING_TOOLS=()

command -v gcc >/dev/null 2>&1 || MISSING_TOOLS+=("gcc")
command -v nasm >/dev/null 2>&1 || MISSING_TOOLS+=("nasm")
command -v make >/dev/null 2>&1 || MISSING_TOOLS+=("make")
command -v grub-install >/dev/null 2>&1 || MISSING_TOOLS+=("grub-install")
command -v parted >/dev/null 2>&1 || MISSING_TOOLS+=("parted")
command -v qemu-img >/dev/null 2>&1 || MISSING_TOOLS+=("qemu-img")

if [ ${#MISSING_TOOLS[@]} -ne 0 ]; then
    echo "ERROR: Missing required tools: ${MISSING_TOOLS[*]}"
    echo ""
    echo "Please run the setup script first:"
    echo "  ./scripts/wsl-setup.sh"
    exit 1
fi

echo "All required tools found!"
echo ""

# Clean previous build
echo "Cleaning previous build..."
make clean

echo ""
echo "Building bootloader..."
make -C bootloader

echo ""
echo "Building drivers..."
make -C drivers

echo ""
echo "Building kernel..."
make -C kernel

echo ""
echo "Creating disk image..."
make build-image

echo ""
echo "=========================================="
echo "Build Complete!"
echo "=========================================="
echo ""
echo "Disk image created: build/disk.img"
echo ""
echo "To run GladOS:"
echo "  On Windows: ./scripts/run.ps1"
echo "  On WSL:     ./scripts/run.sh"
echo ""
