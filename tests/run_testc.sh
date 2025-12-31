#!/bin/bash
# Run this script to compile and run the test program (test.c) for the chess engine.
echo "Compiling with"
gcc --version
echo "-------------------"
gcc test.c ../src/chess_engine.c ../src/utilities.c -o test.exe -Wall
if [ $? -ne 0 ]; then
    echo "Compilation ERROR"
    exit 1
else
    echo "Compilation OK"
fi
echo "Running executable"
echo "-------------------"
./test.exe

