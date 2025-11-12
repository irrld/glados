# GladOS Run Script for Windows
# Runs GladOS using QEMU on Windows

param(
    [switch]$Debug,
    [string]$Memory = "512M",
    [string]$QemuPath = ""
)

$ErrorActionPreference = "Stop"

Write-Host "==========================================" -ForegroundColor Cyan
Write-Host "Running GladOS" -ForegroundColor Cyan
Write-Host "==========================================" -ForegroundColor Cyan
Write-Host ""

# Check if disk image exists
$diskImage = "build\disk.img"
if (-not (Test-Path $diskImage)) {
    Write-Host "ERROR: Disk image not found at $diskImage" -ForegroundColor Red
    Write-Host ""
    Write-Host "Please build the project first:" -ForegroundColor Yellow
    Write-Host "  .\scripts\build.ps1" -ForegroundColor White
    exit 1
}

# Try to find QEMU
$qemuExecutable = "qemu-system-x86_64.exe"
$qemuLocations = @(
    "$QemuPath\$qemuExecutable",
    "C:\Program Files\qemu\$qemuExecutable",
    "C:\Program Files (x86)\qemu\$qemuExecutable",
    "$env:ProgramFiles\qemu\$qemuExecutable",
    "${env:ProgramFiles(x86)}\qemu\$qemuExecutable",
    "$env:LOCALAPPDATA\Programs\qemu\$qemuExecutable"
)

$qemuFound = $null
foreach ($location in $qemuLocations) {
    if (Test-Path $location) {
        $qemuFound = $location
        break
    }
}

# Also check if it's in PATH
if (-not $qemuFound) {
    try {
        $pathQemu = Get-Command $qemuExecutable -ErrorAction SilentlyContinue
        if ($pathQemu) {
            $qemuFound = $pathQemu.Source
        }
    } catch {
        # Not in PATH
    }
}

if (-not $qemuFound) {
    Write-Host "ERROR: QEMU not found" -ForegroundColor Red
    Write-Host ""
    Write-Host "Please install QEMU for Windows:" -ForegroundColor Yellow
    Write-Host "  1. Download from: https://qemu.weilnetz.de/w64/" -ForegroundColor White
    Write-Host "  2. Install to: C:\Program Files\qemu\" -ForegroundColor White
    Write-Host ""
    Write-Host "Or specify QEMU path with:" -ForegroundColor Yellow
    Write-Host "  .\scripts\run.ps1 -QemuPath 'C:\path\to\qemu'" -ForegroundColor White
    exit 1
}

Write-Host "Found QEMU at: $qemuFound" -ForegroundColor Green
Write-Host ""
Write-Host "QEMU Options:" -ForegroundColor Cyan
Write-Host "  - Memory: $Memory" -ForegroundColor Gray
Write-Host "  - CPU: 1 core" -ForegroundColor Gray
Write-Host "  - Serial: stdio" -ForegroundColor Gray

if ($Debug) {
    Write-Host "  - Debug: Enabled (waiting for GDB on port 1234)" -ForegroundColor Yellow
}

Write-Host ""
Write-Host "Starting GladOS..." -ForegroundColor Green
Write-Host "Press Ctrl+C in the QEMU window to exit" -ForegroundColor Yellow
Write-Host ""

# Build QEMU arguments
$qemuArgs = @(
    "-drive", "format=raw,file=$diskImage",
    "-m", $Memory,
    "-smp", "1",
    "-serial", "stdio"
)

if ($Debug) {
    $qemuArgs += @("-s", "-S")
    Write-Host "Debug mode: QEMU will wait for GDB connection on localhost:1234" -ForegroundColor Yellow
    Write-Host "Connect with: gdb -ex 'target remote localhost:1234'" -ForegroundColor Cyan
    Write-Host ""
}

# Run QEMU
& $qemuFound $qemuArgs

Write-Host ""
Write-Host "GladOS exited" -ForegroundColor Cyan
