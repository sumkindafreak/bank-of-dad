#Requires -Version 5.1
<#
.SYNOPSIS
  Install Bank of Dad LVGL sketch + lv_conf.h to your Arduino folders.

.USAGE
  Right-click -> Run with PowerShell
  OR from repo root:
    powershell -ExecutionPolicy Bypass -File tools\install-arduino-sketch.ps1

  Optional custom sketch path:
    powershell -ExecutionPolicy Bypass -File tools\install-arduino-sketch.ps1 -SketchDir "C:\Users\tjpro\Documents\Arduino\BankOfDadLVGL"
#>
param(
    [string]$SketchDir = "$env:USERPROFILE\Documents\Arduino\BankOfDadLVGL",
    [string]$LibrariesDir = "$env:USERPROFILE\Documents\Arduino\libraries"
)

$ErrorActionPreference = "Stop"
$RepoRoot = Resolve-Path (Join-Path $PSScriptRoot "..")
$SrcSketch = Join-Path $RepoRoot "BankOfDadLVGL"
$SrcLvConf = Join-Path $RepoRoot "arduino-libraries\lv_conf.h"

if (-not (Test-Path $SrcSketch)) {
    Write-Error "BankOfDadLVGL folder not found at: $SrcSketch"
}

Write-Host "Bank of Dad — Arduino install" -ForegroundColor Cyan
Write-Host "  From: $SrcSketch"
Write-Host "  To:   $SketchDir"
Write-Host ""

New-Item -ItemType Directory -Force -Path $SketchDir | Out-Null
Copy-Item -Path "$SrcSketch\*" -Destination $SketchDir -Recurse -Force

# Remove legacy cpp files — code now lives in BankOfDadLVGL.ino
foreach ($legacy in @("touch_lvgl.cpp", "rgb_sync.cpp")) {
    $legacyPath = Join-Path $SketchDir $legacy
    if (Test-Path $legacyPath) {
        Remove-Item $legacyPath -Force
        Write-Host "Removed legacy $legacy (now in BankOfDadLVGL.ino)" -ForegroundColor Yellow
    }
}

if (Test-Path $SrcLvConf) {
    New-Item -ItemType Directory -Force -Path $LibrariesDir | Out-Null
    Copy-Item -Path $SrcLvConf -Destination (Join-Path $LibrariesDir "lv_conf.h") -Force
    Write-Host "Copied lv_conf.h -> $LibrariesDir\lv_conf.h" -ForegroundColor Green
} else {
    Write-Warning "arduino-libraries\lv_conf.h not found — copy BankOfDadLVGL\lv_conf.h manually"
}

# Verify fixed files landed
$checks = @(
    @{ File = "touch_lvgl.h"; Pattern = "TOUCH_ROT_CW_90" },
    @{ File = "touch_lvgl.cpp"; Pattern = "points\[0\]" },
    @{ File = "rgb_sync.h"; Pattern = "#include <stdint.h>" },
    @{ File = "BankOfDadLVGL.ino"; Pattern = "TOUCH_ROT_CW_90" }
)

$ok = $true
foreach ($c in $checks) {
    $path = Join-Path $SketchDir $c.File
    $text = Get-Content $path -Raw
    if ($text -match $c.Pattern) {
        Write-Host "[OK] $($c.File)" -ForegroundColor Green
    } else {
        Write-Host "[FAIL] $($c.File) missing: $($c.Pattern)" -ForegroundColor Red
        $ok = $false
    }
}

if ($ok) {
    Write-Host ""
    Write-Host "Done. Open in Arduino IDE:" -ForegroundColor Cyan
    Write-Host "  $SketchDir\BankOfDadLVGL.ino"
    Write-Host ""
    Write-Host "Then: ESP32S3 Dev Module, OPI PSRAM, 16MB Flash, compile."
} else {
    Write-Error "Install verification failed — check paths and re-run."
}
