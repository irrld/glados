# GladOS Build Instructions

This guide explains how to build and run GladOS on Windows, Linux, and WSL.

## Quick Start (Windows)

If you're on Windows with WSL, just run:

```powershell
.\scripts\build-and-run.ps1
```

This single command will:
1. Check if WSL is set up (and offer to set it up if not)
2. Build the entire OS
3. Launch it in QEMU

## Prerequisites

### Windows Users

You need:
1. **WSL (Windows Subsystem for Linux)** - Ubuntu recommended
2. **QEMU for Windows** - Download from https://qemu.weilnetz.de/w64/

#### Installing WSL

Open PowerShell as Administrator and run:
```powershell
wsl --install
```

Restart your computer, then open WSL and set up your username/password.

#### Installing QEMU

1. Download QEMU for Windows from https://qemu.weilnetz.de/w64/
2. Install to `C:\Program Files\qemu\` (or remember the path for later)
3. The scripts will auto-detect QEMU in common locations

### Linux/WSL Users

You need standard build tools. Run the setup script:
```bash
./scripts/wsl-setup.sh
```

This installs:
- GCC cross-compiler
- NASM assembler
- GRUB bootloader tools
- Disk utilities (parted, qemu-img)
- QEMU (optional, for running locally)

## Building

### Option 1: Build and Run (Windows)

One command does everything:
```powershell
.\scripts\build-and-run.ps1
```

Options:
```powershell
# Skip rebuilding (use existing image)
.\scripts\build-and-run.ps1 -SkipBuild

# Debug mode (waits for GDB on port 1234)
.\scripts\build-and-run.ps1 -Debug

# Custom memory
.\scripts\build-and-run.ps1 -Memory 1G

# Custom QEMU path
.\scripts\build-and-run.ps1 -QemuPath "C:\custom\path\qemu"
```

### Option 2: Build Only (Windows)

```powershell
.\scripts\build.ps1
```

This builds the OS using WSL and creates `build\disk.img`.

### Option 3: Linux/WSL Build

```bash
./scripts/build.sh
```

## Running

### Windows

After building:
```powershell
.\scripts\run.ps1
```

Options:
```powershell
# Debug mode
.\scripts\run.ps1 -Debug

# More memory
.\scripts\run.ps1 -Memory 1G

# Custom QEMU path
.\scripts\run.ps1 -QemuPath "C:\path\to\qemu"
```

### Linux/WSL

After building:
```bash
./scripts/run.sh
```

Or use the Makefile:
```bash
make run
```

## Manual Build

If you prefer to build manually:

### WSL/Linux

```bash
# Install dependencies (WSL/Ubuntu)
sudo apt-get update
sudo apt-get install -y build-essential nasm grub-common grub-pc-bin \
    grub-efi-amd64-bin mtools xorriso parted qemu-utils qemu-system-x86

# Build
make clean
make all
make build-image

# Run
qemu-system-x86_64 -drive format=raw,file=build/disk.img -m 512M
```

## Project Structure

```
glados/
├── bootloader/         # GRUB bootloader configuration
├── drivers/           # Device drivers (keyboard, video)
│   ├── keyboard/
│   └── video/
├── kernel/            # Kernel source code
│   ├── include/       # Header files
│   │   └── glados/
│   │       ├── ahci.h        # AHCI driver header
│   │       ├── pci.h         # PCI driver header
│   │       └── ...
│   └── src/           # Implementation files
│       └── glados/
│           ├── ahci.c        # AHCI SATA driver
│           ├── pci.c         # PCI bus driver
│           ├── fs/
│           │   └── ext2.c    # ext2 filesystem
│           └── ...
├── scripts/           # Build and run scripts
│   ├── build.ps1              # Windows build script
│   ├── run.ps1                # Windows run script
│   ├── build-and-run.ps1      # Windows one-command script
│   ├── wsl-setup.sh           # WSL setup script
│   ├── build.sh               # Linux build script
│   └── run.sh                 # Linux run script
├── build/             # Build output (created during build)
│   └── disk.img       # Bootable disk image
└── Makefile           # Main build configuration
```

## Troubleshooting

### "WSL is not available"

Install WSL:
```powershell
wsl --install
```

Then restart your computer.

### "QEMU not found"

**Windows**: Install QEMU from https://qemu.weilnetz.de/w64/ or specify path:
```powershell
.\scripts\run.ps1 -QemuPath "C:\your\qemu\path"
```

**Linux/WSL**: Install QEMU:
```bash
sudo apt-get install qemu-system-x86
```

### "nasm: command not found"

Run the setup script:
```powershell
# Windows
wsl bash scripts/wsl-setup.sh
```

```bash
# Linux/WSL
./scripts/wsl-setup.sh
```

### "Permission denied" on scripts

Make scripts executable:
```bash
chmod +x scripts/*.sh
```

### Build fails with grub errors

Make sure you have GRUB tools installed:
```bash
sudo apt-get install grub-common grub-pc-bin grub-efi-amd64-bin mtools xorriso
```

### QEMU window doesn't appear

Check if the disk image was created:
```bash
ls -lh build/disk.img
```

If it's missing, rebuild:
```powershell
.\scripts\build.ps1
```

## Features

GladOS currently includes:
- **AHCI SATA Driver** - Read/write to SATA disks
- **PCI Bus Driver** - Device enumeration and management
- **ext2 Filesystem** (work in progress)
- **Keyboard Driver** - PS/2 keyboard input
- **Video Driver** - Framebuffer text output
- **Memory Management** - Paging, virtual memory, heap allocation
- **Interrupt Handling** - IDT, PIC, timer
- **Threading** - Basic multitasking support

## Development

### Adding New Features

1. Edit source files in `kernel/src/` or `kernel/include/`
2. Rebuild and test:
   ```powershell
   .\scripts\build-and-run.ps1
   ```

### Debugging

Enable debug mode to use GDB:
```powershell
.\scripts\run.ps1 -Debug
```

In another terminal:
```bash
gdb build/linked.elf
(gdb) target remote localhost:1234
(gdb) continue
```

### Clean Build

```bash
make clean
```

Or on Windows:
```powershell
wsl make clean
```

## License

[Your License Here]
