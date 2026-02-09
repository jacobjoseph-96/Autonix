# Autonix (ADAS Simulation)

![Status](https://img.shields.io/badge/Status-Active-brightgreen)
![C++20](https://img.shields.io/badge/C%2B%2B-20-blue)
![Qt6](https://img.shields.io/badge/Qt-6-green)
![Nix](https://img.shields.io/badge/Nix-Flake-purple)
![Coverage](https://img.shields.io/badge/Coverage-88%25-green)

**Autonix** is a high-fidelity **Advanced Driver Assistance System (ADAS)** simulation environment. Built with **Modern C++20** and **Qt6**, it leverages **NixOS** for a hermetic, reproducible development ecosystem. The project is engineered to strictly adhere to safety-critical standards, including principles from **MISRA C++** and **AUTOSAR**, ensuring robust and reliable simulation capabilities.

## 🚀 Key Features

### 🚗 Vehicle Dynamics & Control
*   **Physics-Based Ego Vehicle**: Accurate simulation of vehicle physics, including heading, speed, and precise lane tracking.
*   **Adaptive Cruise Control (ACC)**: Intelligent speed modulation with three driving modes:
    *   **Eco Mode**: Gentle acceleration (1.5 m/s²), 15m minimum following distance
    *   **Comfort Mode**: Balanced performance (2.0 m/s²), 10m following distance
    *   **Sport Mode**: Aggressive response (3.0 m/s²), 6m following distance
*   **Autonomous Braking System**: Automatic response to traffic controls, red lights, and stop signs.
*   **Predictive Smooth Braking**: Physics-based deceleration logic (`v² / 2d`) ensuring smooth stops exactly 3.0m before stop lines.
*   **Lane Change System**: Safe lane changing with gap analysis and collision avoidance (A/D keys).

### 🚙 NPC Vehicle Traffic
*   **Randomized NPC Spawning**: NPC vehicles spawn at random positions on restart with collision-free placement.
*   **Traffic Rule Compliance**: NPCs obey traffic lights and stop signs realistically (only when enabled).
*   **NPC-to-NPC Collision Avoidance**: Intelligent following behavior prevents vehicle collisions.
*   **Pedestrian Avoidance**: NPCs stop for crossing pedestrians within 5m if pedestrian is in their lane.
*   **Toggle Control**: Enable/disable NPC traffic via dropdown menu - NPCs appear immediately when enabled.

### 🚶 Pedestrian Simulation
*   **Crossing Pedestrians**: Simulates pedestrians crossing roads at intersections.
*   **Emergency Braking**: Ego vehicle automatically brakes when pedestrian detected ahead.
*   **Lane Change Safety**: Pedestrians considered in gap analysis - blocks lane change if pedestrian in target lane.
*   **Randomized Spawn**: Pedestrians spawn at random X positions with random speeds and crossing directions.

### 🚦 Intelligent Traffic System
*   **Dynamic Traffic Lights**: 
    *   Realistic state cycling (Red → Green → Yellow) with configurable timing intervals.
    *   Proximity-aware generation ensuring realistic spacing between intersections.
*   **Smart Traffic Signs**: 
    *   **Verified XML Parser**: Robust scenario loading with full error handling validation.
    *   **Right-Side Enforcement**: Logic to strictly obey signs on the right-hand side of the road.
    *   **Rolling Stop Behavior**: Simulation of realistic "rolling stop" maneuvers (0.5s wait) at stop signs.
*   **Speed Limit Detection**:
    *   **Lane-Specific Enforcement**: Only speed limits in the ego's exact lane are enforced.
    *   **Post-Passing Activation**: Speed limit activates only after passing the sign.
    *   **ACC Integration**: Detected limit automatically caps ACC target speed.
    *   **Visual HUD Widget**: Circular sign-style display with active/inactive status.

### 👁️ Perception & Visualization
*   **3D Perspective Rendering**: Real-time Qt6-based visualization of road geometry, lanes, and environmental elements.
    *   **Interactive Zoom**: Mouse wheel zoom control to inspect traffic details or view the full road.
*   **Field of View (FoV)**: Dynamic visualization of sensor coverage and detection cones.
*   **Active Object Detection**: real-time classification and highlighting of relevant traffic objects.
*   **Heads-Up Overlays**:
    *   **Stop Lines**: Dynamic rendering of stop lines at controlled intersections.
    *   **Detection Bounding Boxes**: Visual indicators for detected objects with relevance status.
*   **Flexible Detection Panel**: Scrollable, dockable detection overlay that can be positioned anywhere.

### 🛠️ Professional Tooling & Quality
*   **Hermetic Build System**: Powered by **Nix Flakes** for pixel-perfect build reproducibility.
*   **Integrated Diagnostics**: Real-time console providing detailed system logs and event tracking.
*   **Robust Testing Suite**: 
    *   **100% Pass Rate**: 15 test suites covering Core, Perception, and Logic.
    *   **UI Testing**: Headless validation of `MainWindow` and UI widgets.
    *   **Code Coverage**: >88% Line Coverage / >94% Function Coverage.
*   **CI/CD Ready**: Automated scripts for artifact generation (JUnit/Cobertura).

---

## ⚡ Quick Start

### Prerequisites
*   **Nix** package manager (with flakes enabled).
*   **WSL2** (if on Windows) or a native Linux environment.

### 1. Build the Project
Use the provided build script to compile the application:
```bash
./build.sh
```

### 2. Run the Simulation
Launch the simulation environment:
```bash
./run.sh
```

### 3. Verification & Testing
Run all unit and integration tests:
```bash
./run_tests.sh
```

Generate a detailed HTML coverage report:
```bash
./check_coverage.sh
```

Generate artifacts for CodeBeamer (JUnit/Cobertura):
```bash
./generate_codebeamer_artifacts.sh
```

---

## 📂 Project Structure

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

For developers who wish to manually interact with the environment:

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

### Toolbar
*   **Start**: Begin the main simulation loop.
*   **Stop**: Pause the simulation.
*   **Reset**: Regenerate the world with new random NPC/pedestrian placements.
*   **Simulation Elements Dropdown**: Toggle visibility and behavior of:
    *   **NPCs**: Enable/disable NPC vehicle traffic.
    *   **Pedestrians**: Enable/disable pedestrian crossing simulation.
    *   **Traffic Lights**: Enable/disable traffic light detection and visualization.
    *   **Stop Signs**: Enable/disable stop sign detection and visualization.
    *   **Stop Lines**: Enable/disable stop line rendering.
*   **ACC (Adaptive Cruise Control)**:
    *   **Checkbox**: Enable/disable ACC system.
    *   **Mode Dropdown**: Select driving mode (Eco, Comfort, Sport).
    *   **Speed Selector**: Set target cruising speed (30-130 km/h).

### Keyboard
*   **A / Left Arrow**: Request lane change to the left.
*   **D / Right Arrow**: Request lane change to the right.
*   **Escape**: Cancel active lane change.

### Diagnostic Logs
*   Monitor real-time system events in the bottom dock.

---

## 🧩 Standards & Compliance

This project is engineered with safety-critical software principles in mind:

*   **C++20 Rules**: Extensive usage of `std::unique_ptr`, `constexpr`, and strong typing.
*   **Compiler Safety**:
    *   `-Wall -Wextra -Wpedantic -Werror` (Treat all warnings as errors)
    *   `-Wconversion -Wsign-conversion` (Prevent implicit numeric conversion bugs)
    *   `-Wshadow` (Prevent variable shadowing)
