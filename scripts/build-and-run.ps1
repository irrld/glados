# GladOS Build and Run Script for Windows
# One command to build and run GladOS

param(
    [switch]$SkipBuild,
    [switch]$Debug,
    [string]$Memory = "512M",
    [string]$QemuPath = ""
)

$ErrorActionPreference = "Stop"

Write-Host ""
Write-Host "==========================================" -ForegroundColor Cyan
Write-Host "  GladOS - Build and Run" -ForegroundColor Cyan
Write-Host "==========================================" -ForegroundColor Cyan
Write-Host ""

if (-not $SkipBuild) {
    # Build the project
    & "$PSScriptRoot\build.ps1"

    if ($LASTEXITCODE -ne 0) {
        Write-Host ""
        Write-Host "Build failed, not starting QEMU" -ForegroundColor Red
        exit 1
    }
} else {
    Write-Host "Skipping build (using existing disk image)" -ForegroundColor Yellow
    Write-Host ""
}

# Small delay to let the user see build completion
Start-Sleep -Milliseconds 500

# Run the project
$runArgs = @{
    Memory = $Memory
}

if ($Debug) {
    $runArgs.Debug = $true
}

if ($QemuPath) {
    $runArgs.QemuPath = $QemuPath
}

& "$PSScriptRoot\run.ps1" @runArgs
