#!/bin/bash
set -e

# Check if cmake is available, if not, try running via nix develop
if ! command -v cmake &> /dev/null; then
    echo "CMake not found. Attempting to run inside 'nix develop'..."
    nix develop --command bash "$0" "$@"
    exit $?
fi

echo "========================================"
echo "    Running ADAS Test Suite"
echo "========================================"

# Configure (Debug build for better test info)
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug

# Build
echo "Building..."
cmake --build build

# Run Tests
echo "Running CTest..."
ctest --test-dir build --output-on-failure

echo "========================================"
echo "    All Tests Completed Successfully"
echo "========================================"
