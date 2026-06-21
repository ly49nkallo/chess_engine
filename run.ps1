#!/usr/bin/env pwsh
$ErrorActionPreference = "Stop"

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
if ([string]::IsNullOrEmpty($scriptDir)) {
    $scriptDir = "."
}
$scriptDir = Resolve-Path $scriptDir

$exeCandidates = @(
    Join-Path $scriptDir "build/CHESS_ENGINE/CHESS_ENGINE.exe"
    Join-Path $scriptDir "build/CHESS_ENGINE/Debug/CHESS_ENGINE.exe"
    Join-Path $scriptDir "build/CHESS_ENGINE/Release/CHESS_ENGINE.exe"
)

$exe = $exeCandidates | Where-Object { Test-Path $_ } | Select-Object -First 1
if (-not $exe) {
    Write-Error "CHESS_ENGINE.exe not found. Run build.ps1 first."
    exit 1
}

$exeDir = Split-Path -Parent $exe
Push-Location $exeDir
try {
    & $exe
} finally {
    Pop-Location
}
