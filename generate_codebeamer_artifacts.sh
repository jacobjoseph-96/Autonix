#!/usr/bin/env bash

# generate_codebeamer_artifacts.sh
# Generates test results (JUnit XML) and coverage reports (Cobertura XML) for CodeBeamer import.

set -e

# Check if cmake is available, if not, try running via nix develop
if ! command -v cmake &> /dev/null; then
    echo "CMake not found. Attempting to run inside 'nix develop'..."
    nix develop --command bash "$0" "$@"
    exit $?
fi

# Configuration
BUILD_DIR="build"
ARTIFACTS_DIR="codebeamer_artifacts"
JUNIT_XML="${ARTIFACTS_DIR}/test_results.xml"
COVERAGE_XML="${ARTIFACTS_DIR}/coverage.xml"

echo "CodeBeamer Artifact Generation"
echo "=============================="

# 1. Clean and Prepare
echo "[1/4] Preparing environment..."
rm -rf "${ARTIFACTS_DIR}"
mkdir -p "${ARTIFACTS_DIR}"

if [ ! -d "${BUILD_DIR}" ]; then
    echo "Build directory not found. Configuring..."
    cmake -B "${BUILD_DIR}" -G Ninja -DENABLE_COVERAGE=ON
else
    # Clean up stale coverage data from previous runs or removed targets
    echo "Cleaning stale coverage data..."
    find "${BUILD_DIR}" -name "*.gcda" -type f -delete
    # Remove stale artifacts from deleted scenario_parser target
    rm -rf "${BUILD_DIR}/CMakeFiles/scenario_parser.dir"
fi

# 2. Build Project
echo "[2/4] Building project..."
cmake --build "${BUILD_DIR}"

# 3. Run Tests (JUnit XML)
echo "[3/4] Running tests and generating JUnit XML..."
# GTEST_OUTPUT must be absolute or relative to CWD.
export GTEST_OUTPUT="xml:$(pwd)/${JUNIT_XML}"

# Run usage ctest to execute all tests
cd "${BUILD_DIR}"
ctest --output-on-failure
cd ..

# Verify JUnit XML exists
if [ -f "${JUNIT_XML}" ]; then
    echo "✅ JUnit XML generated: ${JUNIT_XML}"
else
    echo "⚠️ Warning: JUnit XML not found at ${JUNIT_XML}. Tests might have failed to write output."
fi

# 4. Generate Coverage (Cobertura XML)
echo "[4/4] Generating Code Coverage (xml)..."
cmake --build "${BUILD_DIR}" --target coverage_xml

# Move coverage report to artifacts dir if not already there (gcovr output location depends on configuration)
if [ -f "${BUILD_DIR}/coverage_cobertura.xml" ]; then
    mv "${BUILD_DIR}/coverage_cobertura.xml" "${COVERAGE_XML}"
    echo "✅ Coverage XML generated: ${COVERAGE_XML}"
else
    echo "⚠️ Warning: Coverage XML not found."
fi

echo "=============================="
echo "Artifacts ready in: ${ARTIFACTS_DIR}/"
ls -lh "${ARTIFACTS_DIR}"
echo "=============================="
echo "Import these files into CodeBeamer:"
echo "1. ${JUNIT_XML} (Test Results)"
echo "2. ${COVERAGE_XML} (Coverage Report)"
