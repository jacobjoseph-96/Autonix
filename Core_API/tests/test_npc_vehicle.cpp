#include "npc_vehicle.hpp"
#include "road_segment.hpp"
#include "traffic_light.hpp"
#include "traffic_sign.hpp"
#include <gtest/gtest.h>

using namespace adas::core;

class NPCVehicleTest : public ::testing::Test
{
protected:
  RoadSegment road_{1, 0, 1000};
  std::vector<TrafficLight> lights_;
  std::vector<TrafficSign> signs_;

  void SetUp() override
  {
    road_.addLane(1, LaneBoundary(4.0, 0.0));
  }
};

TEST_F(NPCVehicleTest, RespectsRedLightWithVehicleAhead)
{
  // Scenario: NPC at x=0, speed=10.
  // Red Light at x=10 (Distance 10 < 15 safe distance). Should stop.
  // Vehicle Ahead at x=10 (Gap 10-0-4.5 = 5.5m). Small gap -> reduced speed but
  // > 0.

  NPCVehicle npc(1, 0.0, 1, 10.0);

  // Add Red Light at x=10.0 in lane 1
  lights_.push_back(TrafficLight::create(
      1, Position{10.0, 0.0}, 1, TrafficLightState::Red, 10.0));

  // Vehicle ahead logic passes 'vehicle_ahead_x' = 10.0
  double vehicle_ahead_x = 10.0;

  // Run simulation step
  // Old logic: desired = target(10) * factor(5.5/15) ~ 3.6. Speed reduces
  // towards 3.6. New logic: desired = min(0.0, 3.6) = 0.0. Speed reduces
  // towards 0.0.

  // Perform multiple updates to let speed settle/decrease
  for (int i = 0; i < 20; ++i)
  { // 2 seconds
    npc.update(0.1, road_, lights_, signs_, vehicle_ahead_x);
  }

  // Check speed.
  // If logic holds, speed should be 0.0 (stopped).
  // If bug exists, speed would be around 3.6 (or > 0).

  EXPECT_NEAR(npc.getSpeed(), 0.0, 0.1)
      << "NPC should stop for Red light despite vehicle ahead gap allowing "
         "movement";
}
