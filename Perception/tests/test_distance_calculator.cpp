/**
 * @file test_distance_calculator.cpp
 * @brief GoogleTest suite for SafeDistanceCalculator
 */

#include "types.hpp"
#include "safe_distance_calculator.hpp"
#include <cmath>
#include <gtest/gtest.h>
#include <limits>


using namespace adas::core;
using namespace adas::perception;

namespace {

class DistanceCalculatorTest : public ::testing::Test {
protected:
  void SetUp() override {
    // No setup needed - calculator is stateless
  }
};

// =============================================================================
// Basic Distance Calculations
// =============================================================================

TEST_F(DistanceCalculatorTest, ZeroDistance_SamePoint) {
  const Coordinate p1{0.0, 0.0};
  const Coordinate p2{0.0, 0.0};

  const double distance = SafeDistanceCalculator::calculate(p1, p2);

  EXPECT_DOUBLE_EQ(distance, 0.0);
}

TEST_F(DistanceCalculatorTest, HorizontalDistance) {
  const Coordinate p1{0.0, 0.0};
  const Coordinate p2{10.0, 0.0};

  const double distance = SafeDistanceCalculator::calculate(p1, p2);

  EXPECT_DOUBLE_EQ(distance, 10.0);
}

TEST_F(DistanceCalculatorTest, VerticalDistance) {
  const Coordinate p1{0.0, 0.0};
  const Coordinate p2{0.0, 50.0};

  const double distance = SafeDistanceCalculator::calculate(p1, p2);

  EXPECT_DOUBLE_EQ(distance, 50.0);
}

TEST_F(DistanceCalculatorTest, DiagonalDistance_3_4_5_Triangle) {
  const Coordinate p1{0.0, 0.0};
  const Coordinate p2{3.0, 4.0};

  const double distance = SafeDistanceCalculator::calculate(p1, p2);

  EXPECT_DOUBLE_EQ(distance, 5.0);
}

TEST_F(DistanceCalculatorTest, DiagonalDistance_Arbitrary) {
  const Coordinate p1{1.0, 2.0};
  const Coordinate p2{4.0, 6.0};

  // Distance = sqrt((4-1)^2 + (6-2)^2) = sqrt(9 + 16) = sqrt(25) = 5
  const double distance = SafeDistanceCalculator::calculate(p1, p2);

  EXPECT_DOUBLE_EQ(distance, 5.0);
}

// =============================================================================
// Negative Coordinate Handling
// =============================================================================

TEST_F(DistanceCalculatorTest, NegativeCoordinates) {
  const Coordinate p1{-5.0, -5.0};
  const Coordinate p2{5.0, 5.0};

  // Distance = sqrt(10^2 + 10^2) = sqrt(200) ≈ 14.142
  const double distance = SafeDistanceCalculator::calculate(p1, p2);
  const double expected = std::sqrt(200.0);

  EXPECT_NEAR(distance, expected, 0.001);
}

TEST_F(DistanceCalculatorTest, MixedSignCoordinates) {
  const Coordinate p1{-3.0, 4.0};
  const Coordinate p2{3.0, -4.0};

  // Distance = sqrt(6^2 + 8^2) = sqrt(36 + 64) = sqrt(100) = 10
  const double distance = SafeDistanceCalculator::calculate(p1, p2);

  EXPECT_DOUBLE_EQ(distance, 10.0);
}

// =============================================================================
// Component-wise Distance Calculation
// =============================================================================

TEST_F(DistanceCalculatorTest, CalculateWithRawCoordinates) {
  const double distance = SafeDistanceCalculator::calculate(0.0, 0.0, 3.0, 4.0);

  EXPECT_DOUBLE_EQ(distance, 5.0);
}

// =============================================================================
// Squared Distance (Optimization)
// =============================================================================

TEST_F(DistanceCalculatorTest, SquaredDistance) {
  const Coordinate p1{0.0, 0.0};
  const Coordinate p2{3.0, 4.0};

  const double squared = SafeDistanceCalculator::calculateSquared(p1, p2);

  EXPECT_DOUBLE_EQ(squared, 25.0);
}

// =============================================================================
// Safe Distance with Validation
// =============================================================================

TEST_F(DistanceCalculatorTest, SafeCalculation_ValidInputs) {
  const Coordinate p1{0.0, 0.0};
  const Coordinate p2{3.0, 4.0};

  const auto result = SafeDistanceCalculator::calculateSafe(p1, p2);

  ASSERT_TRUE(result.has_value());
  EXPECT_DOUBLE_EQ(result.value(), 5.0);
}

TEST_F(DistanceCalculatorTest, SafeCalculation_NaN_ReturnsNullopt) {
  const Coordinate p1{std::numeric_limits<double>::quiet_NaN(), 0.0};
  const Coordinate p2{3.0, 4.0};

  const auto result = SafeDistanceCalculator::calculateSafe(p1, p2);

  EXPECT_FALSE(result.has_value());
}

TEST_F(DistanceCalculatorTest, SafeCalculation_Infinity_ReturnsNullopt) {
  const Coordinate p1{0.0, 0.0};
  const Coordinate p2{std::numeric_limits<double>::infinity(), 4.0};

  const auto result = SafeDistanceCalculator::calculateSafe(p1, p2);

  EXPECT_FALSE(result.has_value());
}

// =============================================================================
// Within Distance Check
// =============================================================================

TEST_F(DistanceCalculatorTest, IsWithinDistance_True) {
  const Coordinate p1{0.0, 0.0};
  const Coordinate p2{3.0, 4.0};

  const bool within = SafeDistanceCalculator::isWithinDistance(p1, p2, 10.0);

  EXPECT_TRUE(within);
}

TEST_F(DistanceCalculatorTest, IsWithinDistance_Exactly) {
  const Coordinate p1{0.0, 0.0};
  const Coordinate p2{3.0, 4.0};

  const bool within = SafeDistanceCalculator::isWithinDistance(p1, p2, 5.0);

  EXPECT_TRUE(within);
}

TEST_F(DistanceCalculatorTest, IsWithinDistance_False) {
  const Coordinate p1{0.0, 0.0};
  const Coordinate p2{3.0, 4.0};

  const bool within = SafeDistanceCalculator::isWithinDistance(p1, p2, 4.0);

  EXPECT_FALSE(within);
}

// =============================================================================
// Tolerance Comparison
// =============================================================================

TEST_F(DistanceCalculatorTest, CompareWithTolerance_Exact) {
  const bool result = SafeDistanceCalculator::compareWithTolerance(5.0, 5.0);

  EXPECT_TRUE(result);
}

TEST_F(DistanceCalculatorTest, CompareWithTolerance_WithinDefault) {
  const bool result = SafeDistanceCalculator::compareWithTolerance(5.0005, 5.0);

  EXPECT_TRUE(result);
}

TEST_F(DistanceCalculatorTest, CompareWithTolerance_OutsideDefault) {
  const bool result = SafeDistanceCalculator::compareWithTolerance(5.002, 5.0);

  EXPECT_FALSE(result);
}

TEST_F(DistanceCalculatorTest, CompareWithTolerance_CustomTolerance) {
  const bool result =
      SafeDistanceCalculator::compareWithTolerance(5.05, 5.0, 0.1);

  EXPECT_TRUE(result);
}

// =============================================================================
// Large Distances
// =============================================================================

TEST_F(DistanceCalculatorTest, LargeDistance) {
  const Coordinate p1{0.0, 0.0};
  const Coordinate p2{1000000.0, 1000000.0};

  const double distance = SafeDistanceCalculator::calculate(p1, p2);
  const double expected = std::sqrt(2.0) * 1000000.0;

  EXPECT_NEAR(distance, expected, 1.0); // Within 1 meter for km+ distance
}

// =============================================================================
// Symmetry
// =============================================================================

TEST_F(DistanceCalculatorTest, DistanceIsSymmetric) {
  const Coordinate p1{10.0, 20.0};
  const Coordinate p2{30.0, 40.0};

  const double d1 = SafeDistanceCalculator::calculate(p1, p2);
  const double d2 = SafeDistanceCalculator::calculate(p2, p1);

  EXPECT_DOUBLE_EQ(d1, d2);
}

} // namespace
