/**
 * @file test_road_segment.cpp
 * @brief Unit tests for RoadSegment class
 */

#include "road_segment.hpp"
#include <gtest/gtest.h>

using namespace adas::core;

class RoadSegmentTest : public ::testing::Test
{
protected:
  void SetUp() override
  {
    // 3 lane highway
    // Lane 1: 0-4
    // Lane 2: 4-8
    // Lane 3: 8-12
    road_.addLane(1U, LaneBoundary(4.0, 0.0));
    road_.addLane(2U, LaneBoundary(8.0, 4.0));
    road_.addLane(3U, LaneBoundary(12.0, 8.0));
  }

  RoadSegment road_{1U, 0.0, 100.0};
};

TEST_F(RoadSegmentTest, ConstructorAndGetters)
{
  EXPECT_EQ(road_.getId(), 1U);
  EXPECT_DOUBLE_EQ(road_.getStartX(), 0.0);
  EXPECT_DOUBLE_EQ(road_.getEndX(), 100.0);
}

TEST_F(RoadSegmentTest, AddAndGetLaneBoundary)
{
  auto b1 = road_.getLaneBoundary(1U);
  ASSERT_TRUE(b1.has_value());
  EXPECT_DOUBLE_EQ(b1->left_edge_y, 4.0);
  EXPECT_DOUBLE_EQ(b1->right_edge_y, 0.0);
  EXPECT_DOUBLE_EQ(b1->centerY(), 2.0);

  auto b_missing = road_.getLaneBoundary(99U);
  EXPECT_FALSE(b_missing.has_value());
}

TEST_F(RoadSegmentTest, LaneBoundaryContainsY)
{
  // Test containsY() used by NPC pedestrian avoidance logic
  auto b1 = road_.getLaneBoundary(1U);
  ASSERT_TRUE(b1.has_value());

  // Center of lane 1 (0-4) should be contained
  EXPECT_TRUE(b1->containsY(2.0));

  // Edges should be contained (inclusive)
  EXPECT_TRUE(b1->containsY(0.0));
  EXPECT_TRUE(b1->containsY(4.0));

  // Outside lane should not be contained
  EXPECT_FALSE(b1->containsY(-1.0));
  EXPECT_FALSE(b1->containsY(5.0));
}

TEST_F(RoadSegmentTest, OverwriteLane)
{
  road_.addLane(1U, LaneBoundary(5.0, 0.0)); // Change width
  auto b1 = road_.getLaneBoundary(1U);
  ASSERT_TRUE(b1.has_value());
  EXPECT_DOUBLE_EQ(b1->left_edge_y, 5.0);
}

TEST_F(RoadSegmentTest, FindLaneAtY)
{
  // Center of lanes
  EXPECT_EQ(road_.findLaneAtY(2.0), 1U);
  EXPECT_EQ(road_.findLaneAtY(6.0), 2U);
  EXPECT_EQ(road_.findLaneAtY(10.0), 3U);

  // Edges (inclusive)
  EXPECT_TRUE(road_.findLaneAtY(4.0).has_value()); // Shared edge

  // Outside
  EXPECT_FALSE(road_.findLaneAtY(-1.0).has_value());
  EXPECT_FALSE(road_.findLaneAtY(13.0).has_value());
}

TEST_F(RoadSegmentTest, ContainsPosition)
{
  // Valid X, Valid Y
  EXPECT_TRUE(road_.containsPosition(Position{50.0, 2.0}));

  // Invalid X
  EXPECT_FALSE(road_.containsPosition(Position{-10.0, 2.0}));
  EXPECT_FALSE(road_.containsPosition(Position{110.0, 2.0}));

  // Invalid Y
  EXPECT_FALSE(road_.containsPosition(Position{50.0, 20.0}));
}

TEST_F(RoadSegmentTest, GetLaneIds)
{
  auto ids = road_.getLaneIds();
  EXPECT_EQ(ids.size(), 3U);
  // Order relies on insertion, so 1,2,3
  EXPECT_EQ(ids[0], 1U);
  EXPECT_EQ(ids[1], 2U);
  EXPECT_EQ(ids[2], 3U);
}

TEST_F(RoadSegmentTest, AreLanesAdjacent)
{
  EXPECT_TRUE(road_.areLanesAdjacent(1U, 2U));
  EXPECT_TRUE(road_.areLanesAdjacent(2U, 1U));
  EXPECT_TRUE(road_.areLanesAdjacent(2U, 3U));

  EXPECT_FALSE(road_.areLanesAdjacent(1U, 3U));  // Not adjacent
  EXPECT_FALSE(road_.areLanesAdjacent(1U, 99U)); // Invalid lane
}
