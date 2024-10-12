#!/bin/bash
CWD=$(pwd)
cmake --build $CWD/build --config Debug --target ALL_BUILD -j 18 --
