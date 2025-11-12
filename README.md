# GladOS

A custom operating system kernel with AHCI SATA driver support and ext2 filesystem.

## Quick Start

### Windows Users

```powershell
# One command to build and run!
.\scripts\build-and-run.ps1
```

The script will automatically:
- Check and set up WSL if needed
- Install all dependencies
- Build the OS
- Launch it in QEMU

### Linux/WSL Users

```bash
# First time setup
./scripts/wsl-setup.sh

# Build and run
./scripts/build.sh
./scripts/run.sh
```

## Features

- **AHCI SATA Driver** - Full read/write support for SATA disks via AHCI
- **PCI Bus Driver** - Device enumeration and configuration
- **ext2 Filesystem** - Work in progress
- **Memory Management** - Paging, virtual memory, kernel heap
- **Device Drivers** - Keyboard (PS/2), Video (framebuffer)
- **Interrupts** - IDT, PIC, timer interrupts
- **Threading** - Basic multitasking

## Documentation

See [BUILD.md](BUILD.md) for detailed build instructions and troubleshooting.

## Requirements

- **Windows**: WSL + QEMU
- **Linux**: Build tools, NASM, GRUB, QEMU

All dependencies can be installed automatically with the setup scripts.

## Project Structure

```
glados/
├── kernel/           # OS kernel
│   ├── include/      # Header files
│   └── src/          # Implementation
│       ├── ahci.c    # AHCI SATA driver
│       ├── pci.c     # PCI bus driver
│       └── fs/       # Filesystems
├── drivers/          # Device drivers
├── bootloader/       # GRUB configuration
└── scripts/          # Build automation
```

## Development

### Build Only
```powershell
# Windows
.\scripts\build.ps1

# Linux
./scripts/build.sh
```

### Run Only
```powershell
# Windows
.\scripts\run.ps1

# Linux
./scripts/run.sh
```

### Debug Mode
```powershell
.\scripts\run.ps1 -Debug
```

Then connect with GDB:
```bash
gdb build/linked.elf
(gdb) target remote localhost:1234
```

## Contributing

[Your contributing guidelines here]

## License

[Your license here]
