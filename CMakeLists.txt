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
# Code Coverage
# ============================================================================
option(ENABLE_COVERAGE "Enable code coverage generation" OFF)

if(ENABLE_COVERAGE)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        message(STATUS "Enabling code coverage")
        add_compile_options(--coverage -O1 -g)
        add_link_options(--coverage)
    else()
        message(WARNING "Code coverage not supported for this compiler")
    endif()
endif()

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
add_executable(test_scenarios tests/test_scenarios.cpp)
target_include_directories(test_scenarios PRIVATE
    ${CMAKE_SOURCE_DIR}/Core_API/include
    ${CMAKE_SOURCE_DIR}/Perception/include
)
target_link_libraries(test_scenarios PRIVATE adas_perception adas_core GTest::gtest GTest::gtest_main)
add_test(NAME ScenarioTests COMMAND test_scenarios)

add_executable(test_parser_errors tests/test_parser_errors.cpp)
target_include_directories(test_parser_errors PRIVATE
    ${CMAKE_SOURCE_DIR}/Core_API/include
    ${CMAKE_SOURCE_DIR}/tests
)
target_link_libraries(test_parser_errors PRIVATE adas_core GTest::gtest GTest::gtest_main)
add_test(NAME ParserErrorTests COMMAND test_parser_errors)

add_executable(test_simulation_integration tests/test_simulation_integration.cpp)
target_include_directories(test_simulation_integration PRIVATE
    ${CMAKE_SOURCE_DIR}/Core_API/include
    ${CMAKE_SOURCE_DIR}/Perception/include
)
target_link_libraries(test_simulation_integration PRIVATE adas_perception adas_core GTest::gtest GTest::gtest_main)
add_test(NAME SimulationIntegrationTests COMMAND test_simulation_integration)

add_executable(test_ui_unit tests/test_ui_unit.cpp)
target_include_directories(test_ui_unit PRIVATE
    ${CMAKE_SOURCE_DIR}/UI/include
)
target_link_libraries(test_ui_unit PRIVATE adas_ui Qt6::Widgets GTest::gtest)
add_test(NAME UiUnitTests COMMAND test_ui_unit)

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

if(ENABLE_COVERAGE)
    find_program(LCOV_PATH lcov)
    find_program(GENHTML_PATH genhtml)

    if(LCOV_PATH AND GENHTML_PATH)
        add_custom_target(coverage
            COMMAND ${LCOV_PATH} --capture --directory . --output-file coverage.info
            COMMAND ${LCOV_PATH} --remove coverage.info '/usr/*' '*/tests/*' '/nix/*' '*/build/*' --output-file coverage.cleaned.info
            COMMAND ${GENHTML_PATH} coverage.cleaned.info --output-directory coverage_report
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "Generating code coverage report (HTML)"
        )
    endif()

    find_program(GCOVR_PATH gcovr)
    if(GCOVR_PATH)
        add_custom_target(coverage_xml
            COMMAND ${GCOVR_PATH} --root ${CMAKE_SOURCE_DIR} --xml-pretty --output coverage_cobertura.xml --exclude-directories 'tests' --exclude-directories 'build'
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "Generating code coverage report (Cobertura XML)"
        )
    endif()
endif()
