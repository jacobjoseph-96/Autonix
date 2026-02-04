//! @file test_acc.cpp
//! @brief Unit tests for Adaptive Cruise Control

#include "adaptive_cruise_controller.hpp"
#include <gtest/gtest.h>

using namespace adas::core;

class ACCTest : public ::testing::Test
{
protected:
  void SetUp() override
  {
    // Setup ego vehicle at origin, lane 1
    ego_ = EgoVehicle(Coordinate{0.0, 5.0, 0.0}, 1U);
    ego_.setSpeed(20.0); // 20 m/s = 72 km/h
  }

  EgoVehicle ego_{Coordinate{0.0, 5.0, 0.0}, 1U};
  AdaptiveCruiseController acc_;
};

// ===================================
// Enable/Disable Tests
// ===================================

TEST_F(ACCTest, DisabledByDefault)
{
  EXPECT_FALSE(acc_.isEnabled());
}

TEST_F(ACCTest, EnableDisable)
{
  acc_.enable();
  EXPECT_TRUE(acc_.isEnabled());

  acc_.disable();
  EXPECT_FALSE(acc_.isEnabled());
}

TEST_F(ACCTest, DisabledReturnsCurrentSpeed)
{
  // ACC disabled - should return current ego speed
  double target = acc_.computeTargetSpeed(ego_, std::nullopt, 0.0);
  EXPECT_DOUBLE_EQ(target, 20.0);
}

// ===================================
// Mode Tests
// ===================================

TEST_F(ACCTest, DefaultModeIsComfort)
{
  EXPECT_EQ(acc_.getMode(), ACCMode::Comfort);
}

TEST_F(ACCTest, SetModeUpdatesConfig)
{
  acc_.setMode(ACCMode::Eco);
  EXPECT_EQ(acc_.getMode(), ACCMode::Eco);
  EXPECT_DOUBLE_EQ(acc_.getConfig().time_gap, 3.0);
  EXPECT_DOUBLE_EQ(acc_.getConfig().max_acceleration, 1.5);

  acc_.setMode(ACCMode::Sport);
  EXPECT_EQ(acc_.getMode(), ACCMode::Sport);
  EXPECT_DOUBLE_EQ(acc_.getConfig().time_gap, 2.0);
  EXPECT_DOUBLE_EQ(acc_.getConfig().max_acceleration, 3.0);
}

TEST_F(ACCTest, EcoModeConfig)
{
  auto config = ACCConfig::forMode(ACCMode::Eco);
  EXPECT_DOUBLE_EQ(config.time_gap, 3.0);
  EXPECT_DOUBLE_EQ(config.max_acceleration, 1.5);
  EXPECT_DOUBLE_EQ(config.max_deceleration, 2.5);
  EXPECT_DOUBLE_EQ(config.min_following_dist, 15.0);
}

TEST_F(ACCTest, ComfortModeConfig)
{
  auto config = ACCConfig::forMode(ACCMode::Comfort);
  EXPECT_DOUBLE_EQ(config.time_gap, 2.5);
  EXPECT_DOUBLE_EQ(config.max_acceleration, 2.0);
  EXPECT_DOUBLE_EQ(config.max_deceleration, 3.5);
  EXPECT_DOUBLE_EQ(config.min_following_dist, 10.0);
}

TEST_F(ACCTest, SportModeConfig)
{
  auto config = ACCConfig::forMode(ACCMode::Sport);
  EXPECT_DOUBLE_EQ(config.time_gap, 2.0);
  EXPECT_DOUBLE_EQ(config.max_acceleration, 3.0);
  EXPECT_DOUBLE_EQ(config.max_deceleration, 4.5);
  EXPECT_DOUBLE_EQ(config.min_following_dist, 6.0);
}

// ===================================
// Target Speed Tests
// ===================================

TEST_F(ACCTest, SetTargetSpeedClamped)
{
  acc_.setTargetSpeed(50.0); // Above max
  EXPECT_DOUBLE_EQ(acc_.getTargetSpeed(),
                   AdaptiveCruiseController::kMaxTargetSpeed);

  acc_.setTargetSpeed(1.0); // Below min
  EXPECT_DOUBLE_EQ(acc_.getTargetSpeed(),
                   AdaptiveCruiseController::kMinTargetSpeed);

  acc_.setTargetSpeed(25.0); // Valid
  EXPECT_DOUBLE_EQ(acc_.getTargetSpeed(), 25.0);
}

TEST_F(ACCTest, SetTargetSpeedKmh)
{
  acc_.setTargetSpeedKmh(80.0); // 80 km/h
  EXPECT_NEAR(acc_.getTargetSpeed(), 22.22, 0.01);
  EXPECT_NEAR(acc_.getTargetSpeedKmh(), 80.0, 0.1);
}

// ===================================
// Speed Computation Tests
// ===================================

TEST_F(ACCTest, NoLeadVehicleCruisesAtTargetSpeed)
{
  acc_.enable();
  acc_.setTargetSpeed(25.0);

  double target = acc_.computeTargetSpeed(ego_, std::nullopt, 0.0);
  EXPECT_DOUBLE_EQ(target, 25.0);
}

TEST_F(ACCTest, LeadVehicleFarAwayCruisesAtTargetSpeed)
{
  acc_.enable();
  acc_.setTargetSpeed(25.0);

  // Lead 200m ahead - well beyond safe gap
  Coordinate lead_pos{200.0, 5.0, 0.0};
  double target = acc_.computeTargetSpeed(ego_, lead_pos, 20.0);
  EXPECT_DOUBLE_EQ(target, 25.0);
}

TEST_F(ACCTest, LeadVehicleCloseMatchesSpeed)
{
  acc_.enable();
  acc_.setTargetSpeed(25.0);
  ego_.setSpeed(20.0);

  // Comfort mode: safe_gap = max(20*2.0, 5) = 40m
  // Lead at 30m - within safe gap
  Coordinate lead_pos{30.0, 5.0, 0.0};
  double target = acc_.computeTargetSpeed(ego_, lead_pos, 15.0);

  // Should match lead speed (15) since within safe gap
  EXPECT_DOUBLE_EQ(target, 15.0);
}

TEST_F(ACCTest, EmergencyStopWhenLeadBehind)
{
  acc_.enable();

  // Lead behind ego (collision state)
  Coordinate lead_pos{-5.0, 5.0, 0.0};
  double target = acc_.computeTargetSpeed(ego_, lead_pos, 0.0);

  EXPECT_DOUBLE_EQ(target, 0.0);
}

TEST_F(ACCTest, VeryCloseSlowsMoreAggressively)
{
  acc_.enable();
  acc_.setTargetSpeed(25.0);
  ego_.setSpeed(20.0);

  // Comfort mode: safe_gap = 40m
  // Lead at 15m - gap_ratio = 15/40 = 0.375 < 0.5
  Coordinate lead_pos{15.0, 5.0, 0.0};
  double target = acc_.computeTargetSpeed(ego_, lead_pos, 10.0);

  // Should slow more aggressively: lead_speed * 0.8 = 8.0
  EXPECT_DOUBLE_EQ(target, 8.0);
}

// ===================================
// Update Tests
// ===================================

TEST_F(ACCTest, UpdateDoesNothingWhenDisabled)
{
  ego_.setSpeed(20.0);
  acc_.update(ego_, std::nullopt, 0.0, 0.1);

  // Speed unchanged since ACC is disabled
  EXPECT_DOUBLE_EQ(ego_.getSpeed(), 20.0);
}

TEST_F(ACCTest, UpdateAppliesAccelerationLimits)
{
  acc_.enable();
  acc_.setMode(ACCMode::Comfort); // max_accel = 2.0 m/s²
  acc_.setTargetSpeed(30.0);
  ego_.setSpeed(20.0);

  // No lead vehicle, should accelerate toward 30 m/s
  // Max increase in 0.1s = 2.0 * 0.1 = 0.2 m/s
  acc_.update(ego_, std::nullopt, 0.0, 0.1);

  EXPECT_DOUBLE_EQ(ego_.getSpeed(), 20.2);
}

TEST_F(ACCTest, UpdateAppliesDecelerationLimits)
{
  acc_.enable();
  acc_.setMode(ACCMode::Comfort); // max_decel = 3.5 m/s²
  ego_.setSpeed(20.0);

  // Lead vehicle very close, emergency slow down
  Coordinate lead_pos{5.0, 5.0, 0.0};

  // Desired speed should be ~0, but limited by deceleration
  // Max decrease in 0.1s = 3.5 * 0.1 = 0.35 m/s
  acc_.update(ego_, lead_pos, 0.0, 0.1);

  EXPECT_DOUBLE_EQ(ego_.getSpeed(), 19.65);
}

// ===================================
// Mode String Conversion Tests
// ===================================

TEST_F(ACCTest, ModeToString)
{
  EXPECT_STREQ(accModeToString(ACCMode::Eco), "Eco");
  EXPECT_STREQ(accModeToString(ACCMode::Comfort), "Comfort");
  EXPECT_STREQ(accModeToString(ACCMode::Sport), "Sport");
}
