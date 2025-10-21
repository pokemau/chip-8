#!/bin/bash

cmake -S . -B build
cmake --build build
./build/ch8 20 2 "$1"


