#!/bin/bash
# WSL Setup Script for GladOS
# This script installs all required dependencies to build GladOS in WSL

set -e

echo "=========================================="
echo "GladOS - WSL Development Setup"
echo "=========================================="
echo ""

# Check if running in WSL
if ! grep -q Microsoft /proc/version 2>/dev/null && ! grep -q WSL /proc/version 2>/dev/null; then
    echo "Warning: This doesn't appear to be WSL. This script is designed for WSL."
    read -p "Continue anyway? (y/n) " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        exit 1
    fi
fi

echo "Updating package lists..."
sudo apt-get update

echo ""
echo "Installing build essentials..."
sudo apt-get install -y build-essential gcc g++ make

echo ""
echo "Installing cross-compilation tools..."
sudo apt-get install -y gcc-multilib g++-multilib

echo ""
echo "Installing NASM assembler..."
sudo apt-get install -y nasm

echo ""
echo "Installing GRUB tools..."
sudo apt-get install -y grub-common grub-pc-bin grub-efi-amd64-bin mtools xorriso

echo ""
echo "Installing disk utilities..."
sudo apt-get install -y parted qemu-utils

echo ""
echo "Installing QEMU (optional, for running in WSL)..."
sudo apt-get install -y qemu-system-x86

echo ""
echo "=========================================="
echo "Setup Complete!"
echo "=========================================="
echo ""
echo "All dependencies have been installed."
echo "You can now build GladOS by running:"
echo "  ./scripts/build.sh"
echo ""
