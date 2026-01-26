/**
 * @file test_perception_filter.cpp
 * @brief GoogleTest suite for PerceptionFilter
 */

#include "ego_vehicle.hpp"
#include "road_segment.hpp"
#include "traffic_sign.hpp"
#include "perception_filter.hpp"
#include <cmath>
#include <gtest/gtest.h>


using namespace adas::core;
using namespace adas::perception;

namespace {

class PerceptionFilterTest : public ::testing::Test {
protected:
  void SetUp() override {
    // Create road with 3 lanes
    road_.addLane(1U, LaneBoundary{10.0, 5.0}); // Lane 1
    road_.addLane(2U, LaneBoundary{5.0, 0.0});  // Lane 2
    road_.addLane(3U, LaneBoundary{0.0, -5.0}); // Lane 3
  }

  PerceptionFilter filter_;
  RoadSegment road_{1U, 0.0, 100.0};
};

// =============================================================================
// Field of View Tests
// =============================================================================

TEST_F(PerceptionFilterTest, SignDirectlyAhead_InFov) {
  EgoVehicle ego(Coordinate{0.0, 5.0, 0.0}, 1U); // Heading = 0 (east)

  const bool in_fov = filter_.isInFieldOfView(ego, Position{50.0, 5.0});

  EXPECT_TRUE(in_fov);
}

TEST_F(PerceptionFilterTest, SignBehind_NotInFov) {
  EgoVehicle ego(Coordinate{50.0, 5.0, 0.0}, 1U); // Heading = 0 (east)

  const bool in_fov = filter_.isInFieldOfView(ego, Position{0.0, 5.0});

  EXPECT_FALSE(in_fov);
}

TEST_F(PerceptionFilterTest, SignAt60Degrees_InFov) {
  // FoV is 120°, so ±60° should be just inside
  EgoVehicle ego(Coordinate{0.0, 0.0, 0.0}, 1U);

  // Position at 59° from heading
  const double angle = 59.0 * M_PI / 180.0;
  const double x = 50.0 * std::cos(angle);
  const double y = 50.0 * std::sin(angle);

  const bool in_fov = filter_.isInFieldOfView(ego, Position{x, y});

  EXPECT_TRUE(in_fov);
}

TEST_F(PerceptionFilterTest, SignAt61Degrees_NotInFov) {
  // FoV is 120°, so ±60° is the boundary
  EgoVehicle ego(Coordinate{0.0, 0.0, 0.0}, 1U);

  // Position at 61° from heading
  const double angle = 61.0 * M_PI / 180.0;
  const double x = 50.0 * std::cos(angle);
  const double y = 50.0 * std::sin(angle);

  const bool in_fov = filter_.isInFieldOfView(ego, Position{x, y});

  EXPECT_FALSE(in_fov);
}

TEST_F(PerceptionFilterTest, SignAtNegativeAngle_InFov) {
  EgoVehicle ego(Coordinate{0.0, 0.0, 0.0}, 1U);

  // Position at -45° (below heading)
  const double angle = -45.0 * M_PI / 180.0;
  const double x = 50.0 * std::cos(angle);
  const double y = 50.0 * std::sin(angle);

  const bool in_fov = filter_.isInFieldOfView(ego, Position{x, y});

  EXPECT_TRUE(in_fov);
}

TEST_F(PerceptionFilterTest, FovWithRotatedHeading) {
  // Vehicle heading north (90°)
  EgoVehicle ego(Coordinate{0.0, 0.0, M_PI / 2.0}, 1U);

  // Sign directly north - should be in FoV
  const bool in_fov = filter_.isInFieldOfView(ego, Position{0.0, 50.0});

  EXPECT_TRUE(in_fov);
}

TEST_F(PerceptionFilterTest, FovWithRotatedHeading_SignEast) {
  // Vehicle heading north (90°)
  EgoVehicle ego(Coordinate{0.0, 0.0, M_PI / 2.0}, 1U);

  // Sign directly east (90° from heading) - should NOT be in FoV
  const bool in_fov = filter_.isInFieldOfView(ego, Position{50.0, 0.0});

  EXPECT_FALSE(in_fov);
}

// =============================================================================
// Lane Relevance Tests
// =============================================================================

TEST_F(PerceptionFilterTest, SameLane_IsRelevant) {
  const bool relevant = filter_.isLaneRelevant(1U, 1U, road_);

  EXPECT_TRUE(relevant);
}

TEST_F(PerceptionFilterTest, AdjacentLane_IsRelevant) {
  const bool relevant = filter_.isLaneRelevant(1U, 2U, road_);

  EXPECT_TRUE(relevant);
}

TEST_F(PerceptionFilterTest, NonAdjacentLane_NotRelevant) {
  const bool relevant = filter_.isLaneRelevant(1U, 3U, road_);

  EXPECT_FALSE(relevant);
}

// =============================================================================
// Filter Integration Tests
// =============================================================================

TEST_F(PerceptionFilterTest, FilterReturnsSignsInFov) {
  EgoVehicle ego(Coordinate{0.0, 5.0, 0.0}, 1U);

  std::vector<TrafficSign> signs;
  signs.push_back(
      TrafficSign::create(1U, TrafficSignType::Stop, Position{50.0, 5.0}, 1U));
  signs.push_back(TrafficSign::create(2U, TrafficSignType::Yield,
                                      Position{-50.0, 5.0}, 1U)); // Behind

  const auto results = filter_.filter(ego, signs, road_);

  ASSERT_EQ(results.size(), 1U);
  EXPECT_EQ(results[0].sign.getId(), 1U);
}

TEST_F(PerceptionFilterTest, FilterCalculatesDistance) {
  EgoVehicle ego(Coordinate{0.0, 0.0, 0.0}, 1U);

  std::vector<TrafficSign> signs;
  signs.push_back(
      TrafficSign::create(1U, TrafficSignType::Stop, Position{30.0, 40.0}, 1U));

  const auto results = filter_.filter(ego, signs, road_);

  ASSERT_EQ(results.size(), 1U);
  EXPECT_NEAR(results[0].distance, 50.0, 0.001);
}

TEST_F(PerceptionFilterTest, FilterMarksRelevance) {
  EgoVehicle ego(Coordinate{0.0, 7.5, 0.0}, 1U); // In lane 1

  std::vector<TrafficSign> signs;
  signs.push_back(TrafficSign::create(1U, TrafficSignType::Stop,
                                      Position{50.0, 7.5}, 1U)); // Same lane
  signs.push_back(TrafficSign::create(2U, TrafficSignType::Yield,
                                      Position{50.0, -2.5},
                                      3U)); // Lane 3 (not adjacent)

  const auto results = filter_.filter(ego, signs, road_);

  ASSERT_EQ(results.size(), 2U);

  // Find results by ID
  bool found_relevant = false;
  bool found_not_relevant = false;
  for (const auto &r : results) {
    if (r.sign.getId() == 1U) {
      EXPECT_TRUE(r.is_relevant);
      found_relevant = true;
    } else if (r.sign.getId() == 2U) {
      EXPECT_FALSE(r.is_relevant);
      found_not_relevant = true;
    }
  }
  EXPECT_TRUE(found_relevant);
  EXPECT_TRUE(found_not_relevant);
}

TEST_F(PerceptionFilterTest, FilterSortsByDistance) {
  EgoVehicle ego(Coordinate{0.0, 5.0, 0.0}, 1U);

  std::vector<TrafficSign> signs;
  signs.push_back(
      TrafficSign::create(1U, TrafficSignType::Stop, Position{80.0, 5.0}, 1U));
  signs.push_back(
      TrafficSign::create(2U, TrafficSignType::Yield, Position{30.0, 5.0}, 1U));
  signs.push_back(TrafficSign::create(3U, TrafficSignType::SpeedLimit,
                                      Position{50.0, 5.0}, 1U));

  const auto results = filter_.filter(ego, signs, road_);

  ASSERT_EQ(results.size(), 3U);
  EXPECT_EQ(results[0].sign.getId(), 2U); // Closest (30m)
  EXPECT_EQ(results[1].sign.getId(), 3U); // Middle (50m)
  EXPECT_EQ(results[2].sign.getId(), 1U); // Farthest (80m)
}

TEST_F(PerceptionFilterTest, FilterExcludesSignsBeyondRange) {
  PerceptionFilter short_range_filter(constants::kFieldOfViewRadians, 50.0);
  EgoVehicle ego(Coordinate{0.0, 5.0, 0.0}, 1U);

  std::vector<TrafficSign> signs;
  signs.push_back(TrafficSign::create(1U, TrafficSignType::Stop,
                                      Position{30.0, 5.0}, 1U)); // In range
  signs.push_back(TrafficSign::create(2U, TrafficSignType::Yield,
                                      Position{80.0, 5.0}, 1U)); // Out of range

  const auto results = short_range_filter.filter(ego, signs, road_);

  ASSERT_EQ(results.size(), 1U);
  EXPECT_EQ(results[0].sign.getId(), 1U);
}

// =============================================================================
// Edge Cases
// =============================================================================

TEST_F(PerceptionFilterTest, EmptySignList_ReturnsEmpty) {
  EgoVehicle ego(Coordinate{0.0, 5.0, 0.0}, 1U);
  std::vector<TrafficSign> empty_signs;

  const auto results = filter_.filter(ego, empty_signs, road_);

  EXPECT_TRUE(results.empty());
}

TEST_F(PerceptionFilterTest, SignAtOrigin) {
  EgoVehicle ego(Coordinate{-10.0, 0.0, 0.0}, 2U); // Heading east

  std::vector<TrafficSign> signs;
  signs.push_back(
      TrafficSign::create(1U, TrafficSignType::Stop, Position{0.0, 0.0}, 2U));

  const auto results = filter_.filter(ego, signs, road_);

  ASSERT_EQ(results.size(), 1U);
  EXPECT_NEAR(results[0].distance, 10.0, 0.001);
}

TEST_F(PerceptionFilterTest, InvalidSign_Filtered) {
  EgoVehicle ego(Coordinate{0.0, 5.0, 0.0}, 1U);

  std::vector<TrafficSign> signs;
  signs.push_back(TrafficSign{}); // Invalid (default constructed)
  signs.push_back(TrafficSign::create(1U, TrafficSignType::Stop,
                                      Position{50.0, 5.0}, 1U)); // Valid

  const auto results = filter_.filter(ego, signs, road_);

  ASSERT_EQ(results.size(), 1U);
  EXPECT_EQ(results[0].sign.getId(), 1U);
}

} // namespace
