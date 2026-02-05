# Autonix Project Documentation

This document outlines the architectural design, structural organization, and implementation journey of the ADAS Simulation project.

## 1. Architectural Design

### 1.1 Modular Architecture
The project follows a **Layered Architecture** pattern to separate concerns and ensure testability:

1.  **Core Layer (`Core_API`)**:
    *   **Responsibility**: Defines the fundamental data models and business rules.
    *   **Characteristics**: Dependency-free (except standard library), pure C++, highly portable.
    *   **Key Components**: `EgoVehicle`, `TrafficLight`, `TrafficSign`, `RoadSegment`, `TrafficLogic`.

2.  **Perception Layer (`Perception`)**:
    *   **Responsibility**: Processes sensor data and abstractions.
    *   **Characteristics**: Depends on `Core_API`. Stateless functional processing.
    *   **Key Components**: `PerceptionFilter` (FoV calculation), `SafeDistanceCalculator` (Safety metrics).

3.  **Presentation/UI Layer (`UI`)**:
    *   **Responsibility**: Visualization and User Interaction.
    *   **Characteristics**: Built with **Qt6**. Depends on `Core_API` and `Perception`. Acts as the Controller/View.
    *   **Key Components**: `MainWindow` (Application controller), `PerspectiveView` (Rendering), `DetectionOverlay` (Augmented Reality UI).

### 1.2 Design Patterns Used
*   **Factory Pattern**: Used in `TrafficSign::create()` and `TrafficLight::create()` to standardize object instantiation.
*   **Observer Pattern**: (Implicit in Qt) Signals and Slots used for UI updates (e.g., timer ticks triggering `onSimulationTick`).
*   **Dependency Injection**: Dependencies like `EgoVehicle` and lists of traffic objects are passed into Logic/Perception functions rather than being hardcoded singletons.
*   **State Machine**:
    *   **Traffic Lights**: Cycles through Red → Green → Yellow.
    *   **Stop Sign Logic**: Rolling stop behavior (Approach → Stop → Wait Timer → Resume).

---

## 2. Project Structure

The directory structure reflects the architectural modularity:

```text
ADAS-Nix-Project/
├── Core_API/              # The Foundation
│   ├── include/           # Public API headers
│   │   ├── traffic_logic.hpp  # Centralized decision making
│   │   └── ...            # Entity headers
│   ├── src/               # Implementation
│   └── tests/             # Unit tests for Core Logic
├── Perception/            # Intelligence Layer
│   ├── include/
│   ├── src/
│   └── tests/             # Tests for math/filtering logic
├── UI/                    # Visualization Layer
│   ├── include/
│   ├── src/
│   └── (No tests)         # UI logic delegated to Core/Perception for testing
├── tests/                 # Integration Tests
│   ├── test_scenarios.cpp # End-to-end XML scenario verification
│   └── ...
├── cmake/                 # Build Configuration
├── COMPLIANCE.md          # Safety Audit & Standards Report
└── flake.nix              # Reproducible Environment Definition
```

---

## 3. Implementation Features

### 3.1 Traffic Control Systems
*   **Smart Traffic Lights**:
    *   **Generation**: Lights are generated with **Proximity Checking** (>20m separation) to avoid unrealistic clustering.
    *   **Logic**: Vehicles stop for Red/Yellow lights explicitly within a 2m buffer.
    *   **Visualization**: Real-time state updates in the 3D perspective view.

*   **Stop Sign Logic (Rolling Stop)**:
    *   **Requirement**: Vehicles must come to a complete stop and wait before proceeding.
    *   **Implementation**:
        *   `TrafficLogic::shouldStopForSign` detects relevant signs.
        *   **State Tracking**: Internal state monitors `wait_timer` (0.5s).
        *   **Once-Per-Sign**: Logic ensures vehicles stop only once per unique sign ID.
        *   **Lane Compliance**: Vehicles ignore stop signs in adjacent lanes.

### 3.2 Visual Aids
*   **Stop Lines**: White stop lines are dynamically drawn at:
    *   Traffic Light positions.
    *   Stop Sign positions.
*   **Field of View**: Semi-transparent cone visualizing the sensor range.
*   **Detection Overlays**: Bounding boxes and distance metrics drawn over detected objects.

---

## 4. Quality Assurance Strategy

### 4.1 Testing Pyramid
1.  **Unit Tests (Core & Perception)**:
    *   `TrafficLogicTests`: Covers 100% of the stop/go decision logic. Verified edge cases like "Sign too far", "Timer expiration", and "Wrong Lane".
    *   `PerceptionFilterTests`: Validates geometric calculations (FoV containment).
    *   `DistanceCalculatorTests`: Validates safety gap checks.
    
2.  **Integration Tests**:
    *   `ScenarioTests`: XML-driven scenarios that simulate discrete world states to verify the system's calculated response (e.g., "Expected Distance" matches).

### 4.2 Standards Compliance
For detailed safety arguments, refer to **[COMPLIANCE.md](file:///wsl.localhost/Ubuntu/home/jacobjoseph/ADAS-Nix-Project/COMPLIANCE.md)**.

*   **MISRA C++ Guidelines**: Enforced via compiler flags (`-Werror`, `-Wconversion`). Code audit confirms no exceptions or raw pointers in Core/Perception.
*   **Modern C++20**: Usage of `constexpr`, `std::optional`, and modern STL algorithms.
*   **ISO 26262**: Achieved 100% unit test coverage for safety-critical logic components (`TrafficLogic`).

---

## 5. Development Workflow
*   **Environment**: Nix Flakes provides a hermetic GCC 13 + Qt6 + CMake environment.
*   **Build System**: CMake with Ninja generator for rapid incremental builds.
*   **Continuous Integration**: Local `ctest` runs ensure no regressions before commit.
