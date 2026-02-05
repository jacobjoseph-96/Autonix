/**
 * @file test_gap_analyzer.cpp
 * @brief GoogleTest suite for GapAnalyzer
 */

#include "gap_analyzer.hpp"
#include <gtest/gtest.h>

using namespace adas::core;
using namespace adas::perception;

namespace
{

class GapAnalyzerTest : public ::testing::Test
{
protected:
  void SetUp() override
  {
    // Ego vehicle in lane 1 at origin
    ego_ = EgoVehicle(Coordinate{0.0, 5.0, 0.0}, 1U);
    ego_.setSpeed(20.0);
  }

  EgoVehicle ego_{Coordinate{0.0, 5.0, 0.0}, 1U};
};

// ===================================
// Basic Analysis Tests
// ===================================

TEST_F(GapAnalyzerTest, EmptyRoadIsSafe)
{
  std::vector<EgoVehicle> others;

  // Analyze target lane 2 (adjacent)
  auto result = GapAnalyzer::analyze(ego_, 2U, others);

  EXPECT_TRUE(result.isSafe());
  EXPECT_TRUE(result.target_lane_exists);
  EXPECT_DOUBLE_EQ(result.front_gap, GapAnalyzer::kInfiniteGap);
  EXPECT_DOUBLE_EQ(result.rear_gap, GapAnalyzer::kInfiniteGap);
}

TEST_F(GapAnalyzerTest, VehicleAheadInTargetLane)
{
  std::vector<EgoVehicle> others;
  // Vehicle ahead at 50m in lane 2
  others.emplace_back(Coordinate{50.0, 0.0, 0.0}, 2U);

  auto result = GapAnalyzer::analyze(ego_, 2U, others);

  EXPECT_TRUE(result.isSafe()); // > 10m gap
  EXPECT_DOUBLE_EQ(result.front_gap, 50.0);
  EXPECT_DOUBLE_EQ(result.rear_gap, GapAnalyzer::kInfiniteGap);
}

TEST_F(GapAnalyzerTest, VehicleBehindInTargetLane)
{
  std::vector<EgoVehicle> others;
  // Vehicle behind at -50m in lane 2
  others.emplace_back(Coordinate{-50.0, 0.0, 0.0}, 2U);

  auto result = GapAnalyzer::analyze(ego_, 2U, others);

  EXPECT_TRUE(result.isSafe()); // > 10m gap
  EXPECT_DOUBLE_EQ(result.front_gap, GapAnalyzer::kInfiniteGap);
  EXPECT_DOUBLE_EQ(result.rear_gap, 50.0);
}

TEST_F(GapAnalyzerTest, UnsafeGapAhead)
{
  std::vector<EgoVehicle> others;
  // Vehicle ahead at 5m in lane 2 (unsafe)
  others.emplace_back(Coordinate{5.0, 0.0, 0.0}, 2U);

  auto result = GapAnalyzer::analyze(ego_, 2U, others);

  EXPECT_FALSE(result.isSafe());
  EXPECT_DOUBLE_EQ(result.front_gap, 5.0);
}

TEST_F(GapAnalyzerTest, UnsafeGapBehind)
{
  std::vector<EgoVehicle> others;
  // Vehicle behind at -5m in lane 2 (unsafe)
  others.emplace_back(Coordinate{-5.0, 0.0, 0.0}, 2U);

  auto result = GapAnalyzer::analyze(ego_, 2U, others);

  EXPECT_FALSE(result.isSafe());
  EXPECT_DOUBLE_EQ(result.rear_gap, 5.0);
}

// ===================================
// Lane Logic Tests
// ===================================

TEST_F(GapAnalyzerTest, IgnoresVehiclesInOtherLanes)
{
  std::vector<EgoVehicle> others;
  // Vehicle in lane 1 (ego lane) - should be ignored for gap logic in lane 2
  others.emplace_back(Coordinate{5.0, 5.0, 0.0}, 1U);
  // Vehicle in lane 3 - should be ignored
  others.emplace_back(Coordinate{5.0, -5.0, 0.0}, 3U);

  auto result = GapAnalyzer::analyze(ego_, 2U, others);

  EXPECT_TRUE(result.isSafe());
  EXPECT_DOUBLE_EQ(result.front_gap, GapAnalyzer::kInfiniteGap);
}

TEST_F(GapAnalyzerTest, CustomMinGap)
{
  std::vector<EgoVehicle> others;
  // Vehicle ahead at 15m
  others.emplace_back(Coordinate{15.0, 0.0, 0.0}, 2U);

  // Default is 10m, so 15m is safe
  EXPECT_TRUE(GapAnalyzer::analyze(ego_, 2U, others).isSafe());

  // Custom min gap 20m, so 15m is unsafe
  EXPECT_FALSE(GapAnalyzer::analyze(ego_, 2U, others, 20.0).isSafe());
}

// ===================================
// Time To Collision Tests
// ===================================

TEST_F(GapAnalyzerTest, TTC_Closing)
{
  // Gap 20m, closing speed 10 m/s
  double ttc = GapAnalyzer::calculateTimeToCollision(ego_, 20.0, 10.0);
  EXPECT_DOUBLE_EQ(ttc, 2.0);
}

TEST_F(GapAnalyzerTest, TTC_Opening)
{
  // Gap 20m, opening speed -5 m/s (moving away)
  double ttc = GapAnalyzer::calculateTimeToCollision(ego_, 20.0, -5.0);
  EXPECT_DOUBLE_EQ(ttc, std::numeric_limits<double>::infinity());
}

TEST_F(GapAnalyzerTest, TTC_Static)
{
  // Gap 20m, relative speed 0
  double ttc = GapAnalyzer::calculateTimeToCollision(ego_, 20.0, 0.0);
  EXPECT_DOUBLE_EQ(ttc, std::numeric_limits<double>::infinity());
}

} // namespace
