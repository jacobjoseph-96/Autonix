# Autonix (ADAS Simulation)

![Status](https://img.shields.io/badge/Status-Active-brightgreen)
![C++20](https://img.shields.io/badge/C%2B%2B-20-blue)
![Qt6](https://img.shields.io/badge/Qt-6-green)
![Nix](https://img.shields.io/badge/Nix-Flake-purple)

**Autonix** is a high-fidelity **Advanced Driver Assistance System (ADAS)** simulation environment. Built with **Modern C++20** and **Qt6**, it leverages **NixOS** for a hermetic, reproducible development ecosystem. The project is engineered to strictly adhere to safety-critical standards, including principles from **MISRA C++** and **AUTOSAR**, ensuring robust and reliable simulation capabilities.

## 🚀 Key Features

### 🚗 Vehicle Dynamics & Control
*   **Physics-Based Ego Vehicle**: Accurate simulation of vehicle physics, including heading, speed, and precise lane tracking.
*   **Adaptive Cruise Control (ACC)**: Intelligent speed modulation maintaining safe following distances based on lead vehicles.
*   **Autonomous Braking System**: Automatic response to traffic controls, red lights, and stop signs.

### 🚦 Intelligent Traffic System
*   **Dynamic Traffic Lights**: 
    *   Realistic state cycling (Red → Green → Yellow) with configurable timing intervals.
    *   Proximity-aware generation ensuring realistic spacing between intersections.
*   **Smart Traffic Signs**: 
    *   **Right-Side Enforcement**: Logic to strictly obey signs on the right-hand side of the road while ignoring irrelevant signs on the left.
    *   **Rolling Stop Behavior**: Simulation of realistic "rolling stop" maneuvers (0.5s wait) at stop signs.
    *   **State Memory**: Intelligent tracking to prevent repeated stops for the same sign.

### 👁️ Perception & Visualization
*   **3D Perspective Rendering**: Real-time Qt6-based visualization of road geometry, lanes, and environmental elements.
*   **Field of View (FoV)**: Dynamic visualization of sensor coverage and detection cones.
*   **Active Object Detection**: real-time classification and highlighting of relevant traffic objects.
*   **Heads-Up Overlays**:
    *   **Stop Lines**: Dynamic rendering of stop lines at controlled intersections.
    *   **Detection Bounding Boxes**: Visual indicators for detected objects with relevance status.

### 🛠️ Professional Tooling
*   **Hermetic Build System**: Powered by **Nix Flakes** for pixel-perfect build reproducibility across environments.
*   **Integrated Diagnostics**: Real-time console providing detailed system logs and event tracking.
*   **Robust Testing Suite**: Comprehensive unit and integration testing using **GoogleTest**.

---

## ⚡ Quick Start

### Prerequisites
*   **Nix** package manager (with flakes enabled).
*   **WSL2** (if on Windows) or a native Linux environment.

### 1. Build the Project
Use the provided build script to compile the application and tests within the Nix environment:
```bash
./build.sh
```

### 2. Run the Simulation
Launch the simulation environment (handles all necessary Qt environment variables):
```bash
./run.sh
```

---

## 📂 Project Structure

The project is architected with modularity and scalability in mind:

```text
Autonix/
├── Core_API/              # Shared data models, types, and core logic
│   ├── include/           # Public headers (EgoVehicle, TrafficLight, etc.)
│   └── src/               # Implementation logic
├── Perception/            # Sensor fusion algorithms and filtering logic
│   ├── include/           # Perception headers (Filter, SafeDistance)
│   └── src/               # Implementation
├── UI/                    # Qt6 Graphical User Interface
│   ├── include/           # MainWindow, PerspectiveView, Overlay headers
│   └── src/               # Implementation
├── cmake/                 # CMake modules and build configuration
├── tests/                 # Integration scenarios and XML parsers
├── flake.nix              # Nix environment definition
└── CMakeLists.txt         # Root build configuration
```

---

## 🛠️ Development Workflow

For developers who wish to manually interact with the environment or run specific tests:

### Enter Development Shell
```bash
nix develop
```
*This drops you into a shell with GCC 13, Qt6, CMake, Ninja, and all dependencies pre-configured.*

### Manual Build
```bash
cmake -B build -G Ninja
cmake --build build
```

### Run Tests
```bash
ctest --test-dir build --output-on-failure
```

---

## 🎮 Simulation Controls

*   **Start**: Begin the main simulation loop.
*   **Stop**: Pause the simulation.
*   **Reset**: Regenerate the world with new random traffic placements.
*   **Diagnostic Logs**: Monitor real-time system events in the bottom dock.

---

## 🧩 Standards & Compliance

This project is engineered with safety-critical software principles in mind:

*   **C++20 Rules**: Extensive usage of `std::unique_ptr`, `constexpr`, and strong typing.
*   **Compiler Safety**:
    *   `-Wall -Wextra -Wpedantic -Werror` (Treat all warnings as errors)
    *   `-Wconversion -Wsign-conversion` (Prevent implicit numeric conversion bugs)
    *   `-Wshadow` (Prevent variable shadowing)
