/**
 * @file test_simulation_integration.cpp
 * @brief Integration tests for full simulation loop logic
 */

#include "adaptive_cruise_controller.hpp"
#include "ego_vehicle.hpp"
#include "lane_change_controller.hpp"
#include "perception_filter.hpp"
#include "road_segment.hpp"
#include "traffic_logic.hpp"
#include <gtest/gtest.h>
#include <memory>
#include <vector>


using namespace adas::core;
using namespace adas::perception;

// Mock object for other vehicles
struct MockVehicle
{
  int id;
  double x;
  double y;
  double vx;
  double vy;
};

class SimulationIntegrationTest : public ::testing::Test
{
protected:
  void SetUp() override
  {
    // Setup environment
    road_ = std::make_unique<RoadSegment>(1U, 0.0, 1000.0);
    road_->addLane(1U, LaneBoundary(4.0, 0.0));
    road_->addLane(2U, LaneBoundary(8.0, 4.0));

    vehicle_ = std::make_unique<EgoVehicle>(Coordinate{0.0, 2.0, 0.0}, 1U);
    vehicle_->setSpeed(20.0); // 20 m/s

    acc_ = std::make_unique<AdaptiveCruiseController>();
    acc_->enable(); // Enable ACC for tests

    lane_controller_ = std::make_unique<LaneChangeController>();
    filter_ = std::make_unique<PerceptionFilter>();
  }

  std::unique_ptr<RoadSegment> road_;
  std::unique_ptr<EgoVehicle> vehicle_;
  std::unique_ptr<AdaptiveCruiseController> acc_;
  std::unique_ptr<LaneChangeController> lane_controller_;
  std::unique_ptr<PerceptionFilter> filter_;
};

TEST_F(SimulationIntegrationTest, FullDriveCycle)
{
  double simulation_time = 0.0;
  const double dt = 0.1;

  // Add some detected objects
  std::vector<MockVehicle> raw_vehicles;
  // Car ahead in same lane
  raw_vehicles.push_back({101, 100.0, 2.0, 15.0, 0.0});

  // Run simulation for 5 seconds (50 steps)
  for (int step = 0; step < 50; ++step)
  {
    simulation_time += dt;

    // Perception (Simple pass-through for logic)
    std::vector<MockVehicle> visible_vehicles = raw_vehicles;

    // ACC Logic
    double min_dist = 1000.0;
    std::optional<Coordinate> lead_pos;

    for (const auto& obj : visible_vehicles)
    {
      double dist = obj.x - vehicle_->getX();
      if (std::abs(obj.y - vehicle_->getY()) < 2.0)
      {
        if (dist > 0 && dist < min_dist)
        {
          min_dist = dist;
          lead_pos = Coordinate{obj.x, obj.y, 0.0};
        }
      }
    }

    double lead_speed = 15.0;
    double target_speed =
        acc_->computeTargetSpeed(*vehicle_, lead_pos, lead_speed);
    vehicle_->setSpeed(target_speed);

    // Lane Change Logic (Execute change at t=2.0)
    if (simulation_time > 2.0 && simulation_time < 2.15 &&
        !vehicle_->isChangingLane())
    {
      GapAnalysisResult gap_res(true, 50.0, 50.0, 20.0, true);
      bool req_success = lane_controller_->requestLaneChange(
          *vehicle_, LaneChangeDirection::Left, *road_, gap_res);
      EXPECT_TRUE(req_success);
      vehicle_->startLaneChange(
          LaneChangeDirection::Left, 2U, vehicle_->getY(), 6.0);
    }

    // Update Vehicle Physics
    if (vehicle_->isChangingLane())
    {
      if (vehicle_->updateLaneChange(dt, 5.0))
      {
        vehicle_->completeLaneChange();
      }
    }
    else
    {
      vehicle_->update(dt);
    }

    // Update raw detections
    for (auto& obj : raw_vehicles)
    {
      obj.x += obj.vx * dt;
    }
  }

  // Verification
  EXPECT_GT(vehicle_->getX(), 0.0);

  if (vehicle_->isChangingLane())
  {
    EXPECT_NEAR(vehicle_->getLaneChangeProgress(), 0.6, 0.15);
  }
}

TEST_F(SimulationIntegrationTest, EmergencyStopTrigger)
{
  vehicle_->setSpeed(30.0);
  std::optional<Coordinate> lead_pos =
      Coordinate{vehicle_->getX() + 10.0, vehicle_->getY(), 0.0};
  double lead_speed = 0.0;
  double target = acc_->computeTargetSpeed(*vehicle_, lead_pos, lead_speed);
  EXPECT_LT(target, vehicle_->getSpeed());
}
