/**
 * @file ego_vehicle.cpp
 * @brief Ego vehicle implementation
 */

#include "ego_vehicle.hpp"
#include <algorithm>
#include <cmath>

namespace adas
{
namespace core
{

EgoVehicle::EgoVehicle(const Coordinate& position, LaneId lane_id) noexcept
    : position_(position), current_lane_id_(lane_id), speed_(0.0)
{
}

void EgoVehicle::setPosition(const Coordinate& position) noexcept
{
  position_ = position;
}

void EgoVehicle::setLaneId(LaneId lane_id) noexcept
{
  current_lane_id_ = lane_id;
}

void EgoVehicle::setSpeed(double speed) noexcept
{
  speed_ = std::clamp(speed, kMinSpeed, kMaxSpeed);
}

void EgoVehicle::update(double delta_time) noexcept
{
  if (delta_time <= 0.0)
  {
    return;
  }

  const double distance = speed_ * delta_time;
  moveForward(distance);
}

void EgoVehicle::moveForward(double distance) noexcept
{
  position_.x += distance * std::cos(position_.theta);
  position_.y += distance * std::sin(position_.theta);
}

void EgoVehicle::rotate(double angle) noexcept
{
  position_.theta += angle;

  // Normalize to [-π, π]
  while (position_.theta > M_PI)
  {
    position_.theta -= 2.0 * M_PI;
  }
  while (position_.theta < -M_PI)
  {
    position_.theta += 2.0 * M_PI;
  }
}

} // namespace core
} // namespace adas
