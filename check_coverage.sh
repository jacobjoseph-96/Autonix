#!/bin/bash
set -e

# Check if cmake is available, if not, try running via nix develop
if ! command -v cmake &> /dev/null; then
    echo "CMake not found. Attempting to run inside 'nix develop'..."
    nix develop --command bash "$0" "$@"
    exit $?
fi

echo "========================================"
echo "    Generating Coverage Report"
echo "========================================"

# Configure with Coverage Enabled
cmake -B build -G Ninja -DENABLE_COVERAGE=ON -DCMAKE_BUILD_TYPE=Debug

# Build
echo "Building..."
cmake --build build

# Run Tests (Required to generate coverage data)
echo "Running Tests..."
ctest --test-dir build

# Generate Report
echo "Processing Coverage Data..."
cmake --build build --target coverage

echo "========================================"
echo "    Coverage Report Ready"
echo "    Location: build/coverage_report/index.html"
echo "========================================"
