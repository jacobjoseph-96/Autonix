#!/bin/bash
set -e

# Build the project using Nix environment
echo "Starting Autonix build..."
nix develop --command bash -c "cmake -B build -G Ninja && cmake --build build"

echo "Build complete."
