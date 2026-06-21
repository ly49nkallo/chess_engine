#!/bin/bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build"

if [ ! -d "$BUILD_DIR" ] || [ ! -f "$BUILD_DIR/CMakeCache.txt" ]; then
    echo "Build directory not configured. Running cmake configure..."
    cmake -S "$SCRIPT_DIR" -B "$BUILD_DIR"
fi

JOBS="${JOBS:-$(nproc 2>/dev/null || echo 4)}"
cmake --build "$BUILD_DIR" --target CHESS_ENGINE --config Debug --parallel "$JOBS"
