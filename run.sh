#!/bin/bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

EXE_CANDIDATES=(
    "$SCRIPT_DIR/build/CHESS_ENGINE/CHESS_ENGINE.exe"
    "$SCRIPT_DIR/build/CHESS_ENGINE/Debug/CHESS_ENGINE.exe"
    "$SCRIPT_DIR/build/CHESS_ENGINE/Release/CHESS_ENGINE.exe"
)

EXE=""
for candidate in "${EXE_CANDIDATES[@]}"; do
    if [ -f "$candidate" ]; then
        EXE="$candidate"
        break
    fi
done

if [ -z "$EXE" ]; then
    echo "Error: CHESS_ENGINE.exe not found." >&2
    echo "Run build.sh first." >&2
    exit 1
fi

cd "$(dirname "$EXE")"
exec ./CHESS_ENGINE.exe
