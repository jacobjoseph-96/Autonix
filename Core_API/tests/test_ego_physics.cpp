/**
 * @file test_ego_physics.cpp
 * @brief Unit tests for EgoVehicle physics and logic
 */

#include "ego_vehicle.hpp"
#include <gtest/gtest.h>
#include <numbers>

using namespace adas::core;

TEST(EgoPhysicsTest, SpeedClamping)
{
  EgoVehicle v(Coordinate{0, 0, 0}, 1U);

  // Normal
  v.setSpeed(20.0);
  EXPECT_DOUBLE_EQ(v.getSpeed(), 20.0);

  // Max (assuming kMaxSpeed is e.g. 50.0 or similar)
  v.setSpeed(1000.0);
  EXPECT_LT(v.getSpeed(), 1000.0); // Should be clamped

  // Min (assuming >= 0)
  v.setSpeed(-10.0);
  EXPECT_GE(v.getSpeed(), 0.0);
}

TEST(EgoPhysicsTest, MovementUpdate)
{
  EgoVehicle v(Coordinate{0, 0, 0}, 1U);
  v.setSpeed(10.0);

  // Move 1 second (distance = 10m)
  v.update(1.0);
  EXPECT_DOUBLE_EQ(v.getX(), 10.0);
  EXPECT_DOUBLE_EQ(v.getY(), 0.0);

  // Move diagonal (45 deg)
  v.setSpeed(1.0);                  // Reset speed for easy calculation
  v.rotate(std::numbers::pi / 4.0); // 45 deg
  v.setPosition(Coordinate{0, 0, std::numbers::pi / 4.0});
  v.update(sqrt(2.0)); // Move sqrt(2) meters -> should be +1, +1

  EXPECT_NEAR(v.getX(), 1.0, 0.001);
  EXPECT_NEAR(v.getY(), 1.0, 0.001);
}

TEST(EgoPhysicsTest, RotationNormalization)
{
  EgoVehicle v(Coordinate{0, 0, 0}, 1U);

  // Rotate 361 degrees (2pi + small)
  v.rotate(2.0 * std::numbers::pi + 0.1);
  EXPECT_NEAR(v.getHeading(), 0.1, 0.001);

  // Rotate -361 (-2pi - small)
  v.rotate(-4.0 * std::numbers::pi - 0.2); // Cumulative
  // Should normalize to within [-pi, pi]
  EXPECT_LE(v.getHeading(), std::numbers::pi);
  EXPECT_GE(v.getHeading(), -std::numbers::pi);
}

TEST(EgoPhysicsTest, LaneChangeLogicGuard)
{
  EgoVehicle v(Coordinate{0, 0, 0}, 1U);

  v.startLaneChange(LaneChangeDirection::Left, 2U, 0.0, 4.0);
  EXPECT_TRUE(v.isChangingLane());

  // Try to start again (should fail/ignore)
  v.startLaneChange(LaneChangeDirection::Right, 1U, 0.0, -4.0);
  EXPECT_EQ(v.getTargetLaneId(), 2U); // Should still be 2
}

TEST(EgoPhysicsTest, UpdateNegativeTime)
{
  EgoVehicle v(Coordinate{0, 0, 0}, 1U);
  v.setSpeed(10.0);
  v.update(-1.0); // Should do nothing
  EXPECT_DOUBLE_EQ(v.getX(), 0.0);
}
