/**
 * @file test_structs.cpp
 * @brief Unit tests for simple Core API structs and classes to ensure full
 * coverage
 */

#include "ego_vehicle.hpp"
#include "pedestrian.hpp"
#include "traffic_light.hpp"
#include "traffic_sign.hpp"
#include <gtest/gtest.h>

using namespace adas::core;

// ===================================
// EgoVehicle Tests
// ===================================

TEST(StructTest, EgoVehicleGettersSetters)
{
  EgoVehicle vehicle(Coordinate{10.0, 5.0, 1.57}, 1U);

  // Test initial state
  EXPECT_DOUBLE_EQ(vehicle.getX(), 10.0);
  EXPECT_DOUBLE_EQ(vehicle.getY(), 5.0);
  EXPECT_DOUBLE_EQ(vehicle.getHeading(), 1.57);
  EXPECT_EQ(vehicle.getLaneId(), 1U);
  EXPECT_DOUBLE_EQ(vehicle.getSpeed(), 0.0);

  // Test setters
  vehicle.setPosition(Coordinate{20.0, 6.0, 3.14});
  EXPECT_DOUBLE_EQ(vehicle.getX(), 20.0);
  EXPECT_DOUBLE_EQ(vehicle.getY(), 6.0);

  vehicle.setSpeed(15.0);
  EXPECT_DOUBLE_EQ(vehicle.getSpeed(), 15.0);

  vehicle.setLaneId(2U);
  EXPECT_EQ(vehicle.getLaneId(), 2U);
}

// ===================================
// Pedestrian Tests
// ===================================

TEST(StructTest, PedestrianGettersSetters)
{
  // Constructor: ID, x, y, target_y, speed
  Pedestrian ped(1U, 5.0, 5.0, -5.0, 1.5);

  EXPECT_EQ(ped.getId(), 1U);
  EXPECT_DOUBLE_EQ(ped.getX(), 5.0);
  EXPECT_DOUBLE_EQ(ped.getY(), 5.0);
  EXPECT_DOUBLE_EQ(ped.getSpeed(), 1.5);

  // Test reset
  ped.reset(10.0, 10.0, -10.0);
  EXPECT_DOUBLE_EQ(ped.getX(), 10.0);
  EXPECT_DOUBLE_EQ(ped.getY(), 10.0);
}

// ===================================
// TrafficLight Tests
// ===================================

TEST(StructTest, TrafficLightGettersSetters)
{
  TrafficLight light = TrafficLight::create(
      1U, Position{50.0, 0.0}, 1U, TrafficLightState::Red, 10.0);

  EXPECT_EQ(light.getId(), 1U);
  EXPECT_DOUBLE_EQ(light.getX(), 50.0);
  EXPECT_EQ(light.getLaneId(), 1U);
  EXPECT_EQ(light.getState(), TrafficLightState::Red);
  EXPECT_TRUE(light.isRed());
  EXPECT_TRUE(light.shouldStop());

  // Test another state creation
  TrafficLight green_light = TrafficLight::create(
      2U, Position{60.0, 0.0}, 1U, TrafficLightState::Green, 10.0);
  EXPECT_EQ(green_light.getState(), TrafficLightState::Green);
  EXPECT_TRUE(green_light.isGreen());
  EXPECT_FALSE(green_light.shouldStop());
}

// ===================================
// TrafficSign Tests
// ===================================

TEST(StructTest, TrafficSignGettersSetters)
{
  TrafficSign sign =
      TrafficSign::create(1U, TrafficSignType::Stop, Position{30.0, 5.0}, 1U);

  EXPECT_EQ(sign.getId(), 1U);
  EXPECT_EQ(sign.getType(), TrafficSignType::Stop);
  EXPECT_DOUBLE_EQ(sign.getPosition().x, 30.0);
  EXPECT_EQ(sign.getLaneId(), 1U);
  EXPECT_STREQ(sign.getTypeString(), "Stop");

  // Test valid
  EXPECT_TRUE(sign.isValid());
}

// ===================================
// Miscellaneous Tests
// ===================================

TEST(StructTest, CoordinateEquality)
{
  Coordinate c1{10.0, 10.0, 0.0};
  Coordinate c2{10.0, 10.0, 0.0};
  Coordinate c3{10.0005, 10.0, 0.0}; // Within tolerance kFloatTolerance (0.001)
  Coordinate c4{11.0, 10.0, 0.0};

  EXPECT_TRUE(c1.equals(c2));
  EXPECT_TRUE(c1.equals(c3));
  EXPECT_FALSE(c1.equals(c4));
}
