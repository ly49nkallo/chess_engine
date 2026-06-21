#!/usr/bin/env pwsh
$ErrorActionPreference = "Stop"

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
if ([string]::IsNullOrEmpty($scriptDir)) {
    $scriptDir = "."
}
$scriptDir = Resolve-Path $scriptDir
$buildDir = Join-Path $scriptDir "build"

if (-not (Test-Path (Join-Path $buildDir "CMakeCache.txt"))) {
    Write-Host "Build directory not configured. Running cmake configure..."
    cmake -S $scriptDir -B $buildDir
}

$jobs = $env:JOBS
if (-not $jobs) {
    $jobs = (Get-CimInstance Win32_ComputerSystem).NumberOfLogicalProcessors
    if (-not $jobs) { $jobs = 4 }
}

cmake --build $buildDir --target CHESS_ENGINE --config Debug --parallel $jobs
