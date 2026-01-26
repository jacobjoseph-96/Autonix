#!/bin/bash

# Run the simulation inside the Nix environment to ensure libraries and Qt plugins are found
echo "Starting Autonix Simulation..."
nix develop --command ./build/UI/adas_simulation
