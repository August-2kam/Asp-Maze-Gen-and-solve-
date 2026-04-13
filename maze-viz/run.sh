#!/usr/bin/env bash
set -e

if [ ! -f build/CMakeCache.txt ]; then
    cmake -S . -B build
fi

cmake --build build
./build/maze_visualizer
