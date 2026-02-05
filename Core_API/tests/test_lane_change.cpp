/**
 * @file test_lane_change.cpp
 * @brief Unit tests for LaneChangeController
 */

#include "ego_vehicle.hpp"
#include "lane_change_controller.hpp"
#include "road_segment.hpp"
#include "types.hpp"
#include <gtest/gtest.h>


using namespace adas::core;

class LaneChangeTest : public ::testing::Test
{
protected:
  void SetUp() override
  {
    // Vehicle in Lane 1, Y=2.0 (Center of 0-4)
    vehicle_ = EgoVehicle(Coordinate{0.0, 2.0, 0.0}, 1U);
    vehicle_.setSpeed(20.0);

    // Lane 1: 0.0 to 4.0
    // Lane 2: 4.0 to 8.0 (Left of Lane 1)
    road_.addLane(1U, LaneBoundary(4.0, 0.0));
    road_.addLane(2U, LaneBoundary(8.0, 4.0));
  }

  EgoVehicle vehicle_{};
  RoadSegment road_{1U, 0.0, 100.0};
  LaneChangeController controller_{};

  // safe, front, rear, min, exists
  GapAnalysisResult safe_gap_{true, 50.0, 50.0, 10.0, true};
  GapAnalysisResult unsafe_gap_{false, 5.0, 50.0, 10.0, true};
};

TEST_F(LaneChangeTest, InitialState)
{
  EXPECT_FALSE(controller_.isActive());
  EXPECT_EQ(controller_.getState(), LaneChangeState::Idle);
}

TEST_F(LaneChangeTest, RequestSuccess)
{
  // Request Left (Lane 1 -> 2)
  bool accepted = controller_.requestLaneChange(
      vehicle_, LaneChangeDirection::Left, road_, safe_gap_);

  EXPECT_TRUE(accepted);
  EXPECT_EQ(controller_.getState(), LaneChangeState::Signaling);
  EXPECT_EQ(vehicle_.getTurnSignal(), TurnSignalState::Left);
}

TEST_F(LaneChangeTest, RequestFailTooSlow)
{
  vehicle_.setSpeed(1.0); // < 2.0
  bool accepted = controller_.requestLaneChange(
      vehicle_, LaneChangeDirection::Left, road_, safe_gap_);

  EXPECT_FALSE(accepted);
  EXPECT_EQ(controller_.getState(), LaneChangeState::Idle);
}

TEST_F(LaneChangeTest, RequestFailUnsafe)
{
  bool accepted = controller_.requestLaneChange(
      vehicle_, LaneChangeDirection::Left, road_, unsafe_gap_);

  EXPECT_FALSE(accepted);
  EXPECT_EQ(controller_.getState(), LaneChangeState::Idle);
}

TEST_F(LaneChangeTest, FullManeuverCycle)
{
  EXPECT_TRUE(controller_.requestLaneChange(
      vehicle_, LaneChangeDirection::Left, road_, safe_gap_));

  // 1. Signaling Phase (0.5s)
  controller_.update(vehicle_, road_, 0.2);
  EXPECT_EQ(controller_.getState(), LaneChangeState::Signaling);

  controller_.update(vehicle_, road_, 0.4); // Total 0.6 > 0.5

  // 2. Executing Phase
  EXPECT_EQ(controller_.getState(), LaneChangeState::Executing);
  EXPECT_TRUE(vehicle_.isChangingLane());

  // 3. Complete Maneuver (2.0s duration)
  // We updated 0.6s total already, need 2.0s more execution
  controller_.update(vehicle_, road_, 1.0);
  EXPECT_EQ(controller_.getState(), LaneChangeState::Executing);

  controller_.update(vehicle_, road_, 1.1); // Total > 2.0s (Exec) -> Completing

  // 3b. Completing Phase (transition tick)
  controller_.update(vehicle_, road_, 0.1);

  // 4. Completing/Idle
  EXPECT_EQ(controller_.getState(), LaneChangeState::Idle);
  EXPECT_FALSE(vehicle_.isChangingLane());
  EXPECT_EQ(vehicle_.getLaneId(), 2U);
  EXPECT_EQ(vehicle_.getTurnSignal(), TurnSignalState::Off);
}

TEST_F(LaneChangeTest, CancelManeuver)
{
  EXPECT_TRUE(controller_.requestLaneChange(
      vehicle_, LaneChangeDirection::Left, road_, safe_gap_));

  controller_.cancel(vehicle_);

  EXPECT_EQ(controller_.getState(), LaneChangeState::Idle);
  EXPECT_EQ(vehicle_.getTurnSignal(), TurnSignalState::Off);
}
