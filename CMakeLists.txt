cmake_minimum_required(VERSION 3.20)
project(Autonix VERSION 1.0.0 LANGUAGES CXX)

# ============================================================================
# C++ Standard Configuration
# ============================================================================
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ============================================================================
# CMake Module Path
# ============================================================================
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

# Include cmake modules
include(CompilerSettings)
include(ClangFormat)
include(ZeroCheck)
include(AllBuild)

# Strict warnings for MISRA compliance
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -Werror")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wconversion -Wsign-conversion")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wcast-qual -Wcast-align")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wshadow -Wnon-virtual-dtor")

# ============================================================================
# Qt6 Configuration
# ============================================================================
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

# ============================================================================
# pugixml for scenario parsing
# ============================================================================
find_package(pugixml REQUIRED)

# ============================================================================
# Testing Configuration
# ============================================================================
enable_testing()
find_package(GTest REQUIRED)

# ============================================================================
# Add Component Subdirectories
# ============================================================================
add_subdirectory(Core_API)
add_subdirectory(Perception)
add_subdirectory(UI)

# ============================================================================
# Integration Tests (Scenario Tests)
# ============================================================================
add_library(scenario_parser STATIC
    tests/xml_scenario_parser.hpp
    tests/xml_scenario_parser.cpp
)
target_include_directories(scenario_parser PUBLIC 
    ${CMAKE_SOURCE_DIR}/Core_API/include
    ${CMAKE_SOURCE_DIR}/Perception/include
    ${CMAKE_SOURCE_DIR}/tests
)
target_link_libraries(scenario_parser PUBLIC adas_core adas_perception pugixml::pugixml)

add_executable(test_scenarios tests/test_scenarios.cpp)
target_include_directories(test_scenarios PRIVATE
    ${CMAKE_SOURCE_DIR}/Core_API/include
    ${CMAKE_SOURCE_DIR}/Perception/include
)
target_link_libraries(test_scenarios PRIVATE scenario_parser adas_perception adas_core GTest::gtest GTest::gtest_main)
add_test(NAME ScenarioTests COMMAND test_scenarios)

# Copy test data
configure_file(${CMAKE_SOURCE_DIR}/tests/test_scenarios.xml 
               ${CMAKE_BINARY_DIR}/test_scenarios.xml COPYONLY)

# ============================================================================
# Custom Targets
# ============================================================================
add_zero_check()
add_all_build()

# ============================================================================
# Installation
# ============================================================================
install(TARGETS adas_simulation DESTINATION bin)
