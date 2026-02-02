/**
 * @file test_pedestrian.cpp
 * @brief Unit tests for Pedestrian class
 */

#include "pedestrian.hpp"
#include <gtest/gtest.h>

namespace adas
{
namespace core
{
namespace test
{

class PedestrianTest : public ::testing::Test
{
protected:
  void SetUp() override
  {
    // Create a default pedestrian for testing
    // Crossing from Y=6.0 to Y=-6.0 (right to left)
    pedestrian_ = Pedestrian(1U, 25.0, 6.0, -6.0, 1.0);
  }

  Pedestrian pedestrian_;
};

// =============================================================================
// Construction Tests
// =============================================================================

TEST_F(PedestrianTest, ConstructorSetsIdCorrectly)
{
  EXPECT_EQ(pedestrian_.getId(), 1U);
}

TEST_F(PedestrianTest, ConstructorSetsPositionCorrectly)
{
  EXPECT_DOUBLE_EQ(pedestrian_.getX(), 25.0);
  EXPECT_DOUBLE_EQ(pedestrian_.getY(), 6.0);
}

TEST_F(PedestrianTest, ConstructorSetsSpeedCorrectly)
{
  EXPECT_DOUBLE_EQ(pedestrian_.getSpeed(), 1.0);
}

TEST_F(PedestrianTest, InitialStateIsWaiting)
{
  EXPECT_EQ(pedestrian_.getState(), PedestrianState::Waiting);
  EXPECT_FALSE(pedestrian_.isCrossing());
}

TEST_F(PedestrianTest, DifferentSpeedValues)
{
  Pedestrian fast_ped(2U, 50.0, 0.0, 10.0, 2.5);
  EXPECT_DOUBLE_EQ(fast_ped.getSpeed(), 2.5);

  Pedestrian slow_ped(3U, 50.0, 0.0, 10.0, 0.5);
  EXPECT_DOUBLE_EQ(slow_ped.getSpeed(), 0.5);
}

// =============================================================================
// State Transition Tests
// =============================================================================

TEST_F(PedestrianTest, StartsWaitingState)
{
  EXPECT_EQ(pedestrian_.getState(), PedestrianState::Waiting);
}

TEST_F(PedestrianTest, TransitionsToCrossingAfterWait)
{
  // Wait time is 2.0 seconds max
  pedestrian_.update(3.5);
  EXPECT_EQ(pedestrian_.getState(), PedestrianState::Crossing);
  EXPECT_TRUE(pedestrian_.isCrossing());
}

TEST_F(PedestrianTest, StaysWaitingDuringWaitPeriod)
{
  pedestrian_.update(1.0);
  EXPECT_EQ(pedestrian_.getState(), PedestrianState::Waiting);
  EXPECT_FALSE(pedestrian_.isCrossing());
}

TEST_F(PedestrianTest, TransitionsToCompletedAfterCrossing)
{
  // Start crossing
  pedestrian_.update(3.5);
  EXPECT_TRUE(pedestrian_.isCrossing());

  // Cross the full distance (12 meters at 1 m/s = 12 seconds)
  pedestrian_.update(15.0);
  EXPECT_EQ(pedestrian_.getState(), PedestrianState::Completed);
  EXPECT_FALSE(pedestrian_.isCrossing());
}

// =============================================================================
// Movement Tests
// =============================================================================

TEST_F(PedestrianTest, PositionChangesWhileCrossing)
{
  // Start crossing
  pedestrian_.update(3.5);
  double initial_y = pedestrian_.getY();

  // Move for 1 second at 1 m/s
  pedestrian_.update(1.0);
  EXPECT_NE(pedestrian_.getY(), initial_y);
}

TEST_F(PedestrianTest, MovesTowardsTarget)
{
  // Pedestrian moves from Y=6.0 to Y=-6.0
  pedestrian_.update(3.5); // Start crossing

  double y1 = pedestrian_.getY();
  pedestrian_.update(1.0);
  double y2 = pedestrian_.getY();

  // Y should be decreasing (moving towards -6.0)
  EXPECT_LT(y2, y1);
}

TEST_F(PedestrianTest, StopsAtTargetPosition)
{
  // Start crossing and move to target
  pedestrian_.update(3.5);  // Start crossing
  pedestrian_.update(15.0); // More than enough time

  // Should stop at or past target
  EXPECT_LE(pedestrian_.getY(), -6.0 + 0.1); // Allow small tolerance
}

TEST_F(PedestrianTest, XPositionRemainsConstant)
{
  double initial_x = pedestrian_.getX();

  pedestrian_.update(3.5); // Start crossing
  pedestrian_.update(5.0); // Cross for a while

  EXPECT_DOUBLE_EQ(pedestrian_.getX(), initial_x);
}

// =============================================================================
// Reset Tests
// =============================================================================

TEST_F(PedestrianTest, ResetResetsPosition)
{
  pedestrian_.update(10.0); // Move somewhere

  pedestrian_.reset(100.0, 5.0, -5.0);

  EXPECT_DOUBLE_EQ(pedestrian_.getX(), 100.0);
  EXPECT_DOUBLE_EQ(pedestrian_.getY(), 5.0);
}

TEST_F(PedestrianTest, ResetResetsStateToWaiting)
{
  pedestrian_.update(3.5); // Start crossing
  EXPECT_EQ(pedestrian_.getState(), PedestrianState::Crossing);

  pedestrian_.reset(50.0, 0.0, 10.0);

  EXPECT_EQ(pedestrian_.getState(), PedestrianState::Waiting);
}

TEST_F(PedestrianTest, ResetPreservesId)
{
  Pedestrian::PedestrianId original_id = pedestrian_.getId();
  pedestrian_.reset(100.0, 0.0, 10.0);
  EXPECT_EQ(pedestrian_.getId(), original_id);
}

// =============================================================================
// Crossing Direction Tests
// =============================================================================

TEST_F(PedestrianTest, CrossesRightToLeft)
{
  Pedestrian ped(1U, 25.0, 6.0, -6.0, 1.0);
  ped.update(3.5); // Start crossing

  double y_before = ped.getY();
  ped.update(1.0);
  double y_after = ped.getY();

  EXPECT_LT(y_after, y_before); // Moving left (decreasing Y)
}

TEST_F(PedestrianTest, CrossesLeftToRight)
{
  Pedestrian ped(2U, 50.0, -6.0, 6.0, 1.0);
  ped.update(3.5); // Start crossing

  double y_before = ped.getY();
  ped.update(1.0);
  double y_after = ped.getY();

  EXPECT_GT(y_after, y_before); // Moving right (increasing Y)
}

// =============================================================================
// isCrossing Tests
// =============================================================================

TEST_F(PedestrianTest, IsCrossingFalseWhenWaiting)
{
  EXPECT_FALSE(pedestrian_.isCrossing());
}

TEST_F(PedestrianTest, IsCrossingTrueWhenCrossing)
{
  pedestrian_.update(3.5);
  EXPECT_TRUE(pedestrian_.isCrossing());
}

TEST_F(PedestrianTest, IsCrossingFalseWhenCompleted)
{
  pedestrian_.update(3.5);  // Start crossing
  pedestrian_.update(15.0); // Complete crossing
  EXPECT_FALSE(pedestrian_.isCrossing());
}

// =============================================================================
// Speed Impact Tests
// =============================================================================

TEST_F(PedestrianTest, FasterPedestrianCrossesQuicker)
{
  Pedestrian slow_ped(1U, 25.0, 6.0, -6.0, 0.5);
  Pedestrian fast_ped(2U, 25.0, 6.0, -6.0, 2.0);

  slow_ped.update(3.5);
  fast_ped.update(3.5);

  slow_ped.update(3.0);
  fast_ped.update(3.0);

  // Fast pedestrian should have moved further
  EXPECT_LT(fast_ped.getY(), slow_ped.getY());
}

// =============================================================================
// State Enum Tests
// =============================================================================

TEST(PedestrianStateTest, AllStatesAreDifferent)
{
  EXPECT_NE(PedestrianState::Waiting, PedestrianState::Crossing);
  EXPECT_NE(PedestrianState::Crossing, PedestrianState::Completed);
  EXPECT_NE(PedestrianState::Waiting, PedestrianState::Completed);
}

} // namespace test
} // namespace core
} // namespace adas
