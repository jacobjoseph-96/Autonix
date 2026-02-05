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

void EgoVehicle::setTurnSignal(TurnSignalState state) noexcept
{
  turn_signal_ = state;
}

void EgoVehicle::startLaneChange(LaneChangeDirection direction,
                                 LaneId target_lane,
                                 double start_y,
                                 double target_y) noexcept
{
  if (is_changing_lane_)
  {
    return; // Already changing lanes
  }

  lane_change_direction_ = direction;
  target_lane_id_ = target_lane;
  lane_change_start_y_ = start_y;
  lane_change_target_y_ = target_y;
  lane_change_progress_ = 0.0;
  is_changing_lane_ = true;

  // Set turn signal
  if (direction == LaneChangeDirection::Left)
  {
    turn_signal_ = TurnSignalState::Left;
  }
  else if (direction == LaneChangeDirection::Right)
  {
    turn_signal_ = TurnSignalState::Right;
  }
}

bool EgoVehicle::updateLaneChange(double delta_time, double duration) noexcept
{
  if (!is_changing_lane_)
  {
    return false;
  }

  // Update progress
  lane_change_progress_ += delta_time / duration;

  if (lane_change_progress_ >= 1.0)
  {
    lane_change_progress_ = 1.0;
    // Smoothly set final position
    position_.y = lane_change_target_y_;
    return true; // Lane change completed
  }

  // Smooth interpolation using easing function (ease-in-out)
  const double t = lane_change_progress_;
  const double smooth_t =
      t < 0.5 ? 2.0 * t * t : 1.0 - std::pow(-2.0 * t + 2.0, 2.0) / 2.0;

  // Interpolate Y position
  position_.y = lane_change_start_y_ +
                (lane_change_target_y_ - lane_change_start_y_) * smooth_t;

  return false;
}

void EgoVehicle::cancelLaneChange() noexcept
{
  is_changing_lane_ = false;
  lane_change_direction_ = LaneChangeDirection::None;
  lane_change_progress_ = 0.0;
  turn_signal_ = TurnSignalState::Off;
}

void EgoVehicle::completeLaneChange() noexcept
{
  current_lane_id_ = target_lane_id_;
  is_changing_lane_ = false;
  lane_change_direction_ = LaneChangeDirection::None;
  lane_change_progress_ = 0.0;
  turn_signal_ = TurnSignalState::Off;
}

} // namespace core
} // namespace adas
