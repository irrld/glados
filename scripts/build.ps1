# GladOS Build Script for Windows
# This script uses WSL to build GladOS and copies the result to Windows

$ErrorActionPreference = "Stop"

Write-Host "==========================================" -ForegroundColor Cyan
Write-Host "Building GladOS (using WSL)" -ForegroundColor Cyan
Write-Host "==========================================" -ForegroundColor Cyan
Write-Host ""

# Check if WSL is available
try {
    $wslVersion = wsl --version 2>&1
    if ($LASTEXITCODE -ne 0) {
        throw "WSL not properly configured"
    }
} catch {
    Write-Host "ERROR: WSL is not available or not properly configured" -ForegroundColor Red
    Write-Host ""
    Write-Host "Please install WSL by running PowerShell as Administrator:" -ForegroundColor Yellow
    Write-Host "  wsl --install" -ForegroundColor Yellow
    Write-Host ""
    Write-Host "Then restart your computer and run this script again." -ForegroundColor Yellow
    exit 1
}

Write-Host "WSL is available" -ForegroundColor Green
Write-Host ""

# Get the current directory in WSL path format
$currentDir = Get-Location
$wslPath = wsl wslpath -a "'$currentDir'"

Write-Host "Current directory (Windows): $currentDir" -ForegroundColor Gray
Write-Host "Current directory (WSL): $wslPath" -ForegroundColor Gray
Write-Host ""

# Check if we're in the GladOS directory
if (-not (Test-Path "Makefile")) {
    Write-Host "ERROR: Not in GladOS root directory" -ForegroundColor Red
    Write-Host "Please run this script from the GladOS root directory" -ForegroundColor Yellow
    exit 1
}

# Check if setup has been run
Write-Host "Checking if WSL is set up..." -ForegroundColor Yellow
$hasNasm = wsl bash -c "command -v nasm >/dev/null 2>&1 && echo 'yes' || echo 'no'"
if ($hasNasm -ne "yes") {
    Write-Host ""
    Write-Host "WSL is not set up with required dependencies" -ForegroundColor Yellow
    Write-Host ""
    $response = Read-Host "Would you like to run the setup script now? (y/n)"
    if ($response -eq "y" -or $response -eq "Y") {
        Write-Host ""
        Write-Host "Running setup script..." -ForegroundColor Cyan
        wsl bash -c "cd '$wslPath' && chmod +x scripts/wsl-setup.sh && ./scripts/wsl-setup.sh"
        Write-Host ""
    } else {
        Write-Host ""
        Write-Host "Please run the setup script manually:" -ForegroundColor Yellow
        Write-Host "  wsl bash scripts/wsl-setup.sh" -ForegroundColor Yellow
        exit 1
    }
}

Write-Host "WSL is properly configured" -ForegroundColor Green
Write-Host ""

# Build the project
Write-Host "Building GladOS..." -ForegroundColor Cyan
Write-Host ""

wsl bash -c "cd '$wslPath' && chmod +x scripts/build.sh && ./scripts/build.sh"

if ($LASTEXITCODE -ne 0) {
    Write-Host ""
    Write-Host "Build failed!" -ForegroundColor Red
    exit 1
}

Write-Host ""
Write-Host "==========================================" -ForegroundColor Green
Write-Host "Build Complete!" -ForegroundColor Green
Write-Host "==========================================" -ForegroundColor Green
Write-Host ""
Write-Host "Disk image: build\disk.img" -ForegroundColor Cyan
Write-Host ""
Write-Host "To run GladOS, execute:" -ForegroundColor Yellow
Write-Host "  .\scripts\run.ps1" -ForegroundColor White
Write-Host ""
