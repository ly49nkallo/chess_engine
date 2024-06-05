#!/bin/bash

gcc test.c ../src/chess_engine.c ../src/utilities.c -o test.exe -Wall

./test.exe

