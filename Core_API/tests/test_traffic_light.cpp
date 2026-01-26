/**
 * @file test_traffic_light.cpp
 * @brief Unit tests for TrafficLight class
 */

#include "traffic_light.hpp"
#include <gtest/gtest.h>

namespace adas {
namespace core {
namespace test {

class TrafficLightTest : public ::testing::Test {
protected:
  void SetUp() override {
    // Create a default traffic light for testing
    light_ = TrafficLight::create(1U, Position{50.0, 7.5}, 1U,
                                  TrafficLightState::Red, 10.0);
  }

  TrafficLight light_;
};

// =============================================================================
// Construction Tests
// =============================================================================

TEST_F(TrafficLightTest, CreateReturnsValidLight) {
  EXPECT_TRUE(light_.isValid());
  EXPECT_EQ(light_.getId(), 1U);
}

TEST_F(TrafficLightTest, DefaultConstructorCreatesInvalidLight) {
  TrafficLight default_light;
  EXPECT_FALSE(default_light.isValid());
}

TEST_F(TrafficLightTest, PositionIsSetCorrectly) {
  EXPECT_DOUBLE_EQ(light_.getX(), 50.0);
  EXPECT_DOUBLE_EQ(light_.getY(), 7.5);
}

TEST_F(TrafficLightTest, LaneIdIsSetCorrectly) {
  EXPECT_EQ(light_.getLaneId(), 1U);
}

TEST_F(TrafficLightTest, InitialStateIsRed) {
  EXPECT_EQ(light_.getState(), TrafficLightState::Red);
  EXPECT_TRUE(light_.isRed());
  EXPECT_FALSE(light_.isYellow());
  EXPECT_FALSE(light_.isGreen());
}

// =============================================================================
// State Query Tests
// =============================================================================

TEST_F(TrafficLightTest, ShouldStopOnRed) {
  auto red_light = TrafficLight::create(2U, Position{0.0, 0.0}, 1U,
                                        TrafficLightState::Red, 10.0);
  EXPECT_TRUE(red_light.shouldStop());
}

TEST_F(TrafficLightTest, ShouldStopOnYellow) {
  auto yellow_light = TrafficLight::create(3U, Position{0.0, 0.0}, 1U,
                                           TrafficLightState::Yellow, 10.0);
  EXPECT_TRUE(yellow_light.shouldStop());
}

TEST_F(TrafficLightTest, ShouldNotStopOnGreen) {
  auto green_light = TrafficLight::create(4U, Position{0.0, 0.0}, 1U,
                                          TrafficLightState::Green, 10.0);
  EXPECT_FALSE(green_light.shouldStop());
}

// =============================================================================
// State Cycling Tests
// =============================================================================

TEST_F(TrafficLightTest, RedTransitionsToGreen) {
  auto red_light = TrafficLight::create(5U, Position{0.0, 0.0}, 1U,
                                        TrafficLightState::Red, 10.0);

  // Red phase is 45% of cycle = 4.5 seconds
  // Update past the red phase
  red_light.update(5.0);

  EXPECT_EQ(red_light.getState(), TrafficLightState::Green);
  EXPECT_TRUE(red_light.isGreen());
}

TEST_F(TrafficLightTest, GreenTransitionsToYellow) {
  auto green_light = TrafficLight::create(6U, Position{0.0, 0.0}, 1U,
                                          TrafficLightState::Green, 10.0);

  // Green phase is 45% of cycle = 4.5 seconds
  green_light.update(5.0);

  EXPECT_EQ(green_light.getState(), TrafficLightState::Yellow);
  EXPECT_TRUE(green_light.isYellow());
}

TEST_F(TrafficLightTest, YellowTransitionsToRed) {
  auto yellow_light = TrafficLight::create(7U, Position{0.0, 0.0}, 1U,
                                           TrafficLightState::Yellow, 10.0);

  // Yellow phase is 10% of cycle = 1.0 second
  yellow_light.update(1.5);

  EXPECT_EQ(yellow_light.getState(), TrafficLightState::Red);
  EXPECT_TRUE(yellow_light.isRed());
}

TEST_F(TrafficLightTest, FullCycleReturnsToOriginalState) {
  auto light = TrafficLight::create(8U, Position{0.0, 0.0}, 1U,
                                    TrafficLightState::Red, 10.0);

  // Full cycle is 10 seconds
  light.update(10.0);

  EXPECT_EQ(light.getState(), TrafficLightState::Red);
}

TEST_F(TrafficLightTest, MultipleCyclesWork) {
  auto light = TrafficLight::create(9U, Position{0.0, 0.0}, 1U,
                                    TrafficLightState::Green, 5.0);

  // 3 full cycles = 15 seconds
  light.update(15.0);

  EXPECT_EQ(light.getState(), TrafficLightState::Green);
}

// =============================================================================
// Time Remaining Tests
// =============================================================================

TEST_F(TrafficLightTest, TimeRemainingDecreasesWithUpdate) {
  auto light = TrafficLight::create(10U, Position{0.0, 0.0}, 1U,
                                    TrafficLightState::Red, 10.0);

  double initial_remaining = light.getTimeRemaining();
  light.update(1.0);
  double after_remaining = light.getTimeRemaining();

  EXPECT_LT(after_remaining, initial_remaining);
}

// =============================================================================
// trafficLightStateToString Tests
// =============================================================================

TEST(TrafficLightStateToStringTest, ReturnsCorrectStrings) {
  EXPECT_STREQ(trafficLightStateToString(TrafficLightState::Red), "Red");
  EXPECT_STREQ(trafficLightStateToString(TrafficLightState::Yellow), "Yellow");
  EXPECT_STREQ(trafficLightStateToString(TrafficLightState::Green), "Green");
}

// =============================================================================
// Different Cycle Durations
// =============================================================================

TEST(TrafficLightCycleDurationTest, ShortCycleWorks) {
  auto fast_light = TrafficLight::create(11U, Position{0.0, 0.0}, 1U,
                                         TrafficLightState::Red, 2.0);

  // Red phase = 0.9s, should transition to green
  fast_light.update(1.0);
  EXPECT_EQ(fast_light.getState(), TrafficLightState::Green);
}

TEST(TrafficLightCycleDurationTest, LongCycleWorks) {
  auto slow_light = TrafficLight::create(12U, Position{0.0, 0.0}, 1U,
                                         TrafficLightState::Red, 20.0);

  // Red phase = 9s, should still be red after 5s
  slow_light.update(5.0);
  EXPECT_EQ(slow_light.getState(), TrafficLightState::Red);
}

} // namespace test
} // namespace core
} // namespace adas
