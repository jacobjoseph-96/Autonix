
#include "traffic_logic.hpp"
#include <gtest/gtest.h>

using namespace adas::core;

class TrafficLogicTest : public ::testing::Test
{
protected:
  void SetUp() override
  {
    // Setup vehicle at origin
    vehicle_ = EgoVehicle(Coordinate{0.0, 5.0, 0.0}, 1U); // Lane 1
  }

  EgoVehicle vehicle_{Coordinate{0.0, 5.0, 0.0}, 1U};
  StopSignState state_{};
};

// ===================================
// Stop Sign Tests
// ===================================

TEST_F(TrafficLogicTest, StopsForSignAhead)
{
  std::vector<TrafficSign> signs;
  signs.push_back(
      TrafficSign::create(1U, TrafficSignType::Stop, Position{10.0, 7.5}, 1U));

  // Move vehicle closer
  vehicle_.setPosition(Coordinate{9.0, 5.0, 0.0});

  // Should stop
  EXPECT_TRUE(TrafficLogic::shouldStopForSign(vehicle_, signs, state_, 0.1));
  EXPECT_TRUE(state_.was_stopped);
  EXPECT_EQ(state_.current_sign_id, 1U);
}

TEST_F(TrafficLogicTest, IgnoresSignTooFar)
{
  std::vector<TrafficSign> signs;
  signs.push_back(
      TrafficSign::create(1U, TrafficSignType::Stop, Position{100.0, 7.5}, 1U));

  EXPECT_FALSE(TrafficLogic::shouldStopForSign(vehicle_, signs, state_, 0.1));
  EXPECT_FALSE(state_.was_stopped);
}

TEST_F(TrafficLogicTest, IgnoresSignWrongLane)
{
  std::vector<TrafficSign> signs;
  signs.push_back(
      TrafficSign::create(1U, TrafficSignType::Stop, Position{10.0, 2.5}, 2U));

  EXPECT_FALSE(TrafficLogic::shouldStopForSign(vehicle_, signs, state_, 0.1));
}

TEST_F(TrafficLogicTest, IgnoresNonStopSign)
{
  std::vector<TrafficSign> signs;
  signs.push_back(TrafficSign::create(
      1U, TrafficSignType::SpeedLimit, Position{10.0, 7.5}, 1U));

  EXPECT_FALSE(TrafficLogic::shouldStopForSign(vehicle_, signs, state_, 0.1));
}

TEST_F(TrafficLogicTest, WaitTimerLogic)
{
  std::vector<TrafficSign> signs;
  signs.push_back(
      TrafficSign::create(1U, TrafficSignType::Stop, Position{10.0, 7.5}, 1U));

  // Move vehicle closer
  vehicle_.setPosition(Coordinate{9.0, 5.0, 0.0});

  // First tick: detects sign, starts waiting
  EXPECT_TRUE(TrafficLogic::shouldStopForSign(vehicle_, signs, state_, 0.1));
  EXPECT_TRUE(state_.was_stopped);
  EXPECT_EQ(state_.wait_timer, TrafficLogic::kStopSignWaitTime);

  // Subsequent ticks: waiting
  state_.wait_timer = 0.2;
  EXPECT_TRUE(TrafficLogic::shouldStopForSign(vehicle_, signs, state_, 0.1));
  EXPECT_DOUBLE_EQ(state_.wait_timer, 0.1);

  // Timer expires
  EXPECT_FALSE(TrafficLogic::shouldStopForSign(vehicle_, signs, state_, 0.1));
  EXPECT_FALSE(state_.was_stopped);
  EXPECT_TRUE(state_.processed_signs.count(1U));
}

TEST_F(TrafficLogicTest, IgnoresProcessedSigns)
{
  std::vector<TrafficSign> signs;
  signs.push_back(
      TrafficSign::create(1U, TrafficSignType::Stop, Position{10.0, 7.5}, 1U));

  state_.processed_signs.insert(1U);

  EXPECT_FALSE(TrafficLogic::shouldStopForSign(vehicle_, signs, state_, 0.1));
}

TEST_F(TrafficLogicTest, StopsForRedLight)
{
  std::vector<TrafficLight> lights;
  lights.push_back(TrafficLight::create(
      1U, Position{10.0, 7.5}, 1U, TrafficLightState::Red, 10.0));

  // Move vehicle closer
  vehicle_.setPosition(Coordinate{9.0, 5.0, 0.0});

  EXPECT_TRUE(TrafficLogic::shouldStopForLight(vehicle_, lights));
}

TEST_F(TrafficLogicTest, StopsForYellowLight)
{
  std::vector<TrafficLight> lights;
  lights.push_back(TrafficLight::create(
      1U, Position{10.0, 7.5}, 1U, TrafficLightState::Yellow, 10.0));

  // Move vehicle closer
  vehicle_.setPosition(Coordinate{9.0, 5.0, 0.0});

  EXPECT_TRUE(TrafficLogic::shouldStopForLight(vehicle_, lights));
}

TEST_F(TrafficLogicTest, IgnoresGreenLight)
{
  std::vector<TrafficLight> lights;
  lights.push_back(TrafficLight::create(
      1U, Position{10.0, 7.5}, 1U, TrafficLightState::Green, 10.0));

  EXPECT_FALSE(TrafficLogic::shouldStopForLight(vehicle_, lights));
}

TEST_F(TrafficLogicTest, IgnoresLightWrongLane)
{
  std::vector<TrafficLight> lights;
  lights.push_back(TrafficLight::create(
      1U, Position{10.0, 2.5}, 2U, TrafficLightState::Red, 10.0));

  EXPECT_FALSE(TrafficLogic::shouldStopForLight(vehicle_, lights));
}

// ===================================
// Generation Tests
// ===================================

TEST_F(TrafficLogicTest, ValidatesLightPosition)
{
  std::vector<TrafficLight> lights;
  // Light at x=20
  lights.push_back(TrafficLight::create(
      1U, Position{20.0, 7.5}, 1U, TrafficLightState::Red, 10.0));

  // Try placing at 25 (too close, delta=5 < 20)
  EXPECT_FALSE(TrafficLogic::isPositionValid(25.0, lights));

  // Try placing at 10 (too close, delta=10 < 20)
  EXPECT_FALSE(TrafficLogic::isPositionValid(10.0, lights));

  // Try placing at 50 (valid, delta=30 >= 20)
  EXPECT_TRUE(TrafficLogic::isPositionValid(50.0, lights));
}

// ===================================
// Adaptive Cruise Control Tests
// ===================================

TEST_F(TrafficLogicTest, MaintainSpeedWithoutLead)
{
  // Lead vehicle very far ahead (1000m)
  LeadVehicle lead(Coordinate{1000.0, 5.0, 0.0}, 1U);
  lead.setSpeed(20.0);

  // Ego at 0, Speed 20. Set Speed 25.
  vehicle_.setPosition(Coordinate{0.0, 5.0, 0.0});
  vehicle_.setSpeed(20.0);

  double target = TrafficLogic::computeACCSpeed(vehicle_, lead, 25.0, 2.0);

  // Should accelerate to set speed
  EXPECT_DOUBLE_EQ(target, 25.0);
}

TEST_F(TrafficLogicTest, MatchSpeedWhenClose)
{
  // Lead vehicle ahead at 30m, Speed 15.
  // Ego at 0, Speed 20.
  // Gap needed: 20 * 2.0 = 40m + 5m buffer = 45m.
  // Actual Gap: 30m.
  // Since 30m < 45m, should match lead speed (15).

  LeadVehicle lead(Coordinate{30.0, 5.0, 0.0}, 1U);
  lead.setSpeed(15.0);

  vehicle_.setPosition(Coordinate{0.0, 5.0, 0.0});
  vehicle_.setSpeed(20.0);

  double target = TrafficLogic::computeACCSpeed(vehicle_, lead, 25.0, 2.0);

  // Should match lead speed
  EXPECT_DOUBLE_EQ(target, 15.0);
}

TEST_F(TrafficLogicTest, EmergencyStopCollision)
{
  // Lead vehicle "behind" (collision state or sensor error)
  LeadVehicle lead(Coordinate{-5.0, 5.0, 0.0}, 1U);
  lead.setSpeed(0.0);

  vehicle_.setPosition(Coordinate{0.0, 5.0, 0.0});

  double target = TrafficLogic::computeACCSpeed(vehicle_, lead, 25.0, 2.0);

  EXPECT_DOUBLE_EQ(target, 0.0);
}

TEST_F(TrafficLogicTest, SafeFollowingDistance)
{
  // Ego Speed 20. Safety Gap = 20*2 + 5 = 45m.
  // Lead at 50m. Spread > Gap.
  // Should maintain set speed (25) if safe.

  LeadVehicle lead(Coordinate{50.0, 5.0, 0.0}, 1U);
  lead.setSpeed(20.0);

  vehicle_.setPosition(Coordinate{0.0, 5.0, 0.0});
  vehicle_.setSpeed(20.0);

  double target = TrafficLogic::computeACCSpeed(vehicle_, lead, 25.0, 2.0);

  EXPECT_DOUBLE_EQ(target, 25.0);
}
