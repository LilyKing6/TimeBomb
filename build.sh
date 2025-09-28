#!/bin/bash

# Create build directory
mkdir build

# Enter build directory
cd build

# Run cmake to generate Makefiles
cmake -G "Unix Makefiles" ..

# Run make to build the project
make -j 8

# Go back to the previous directory
cd ..