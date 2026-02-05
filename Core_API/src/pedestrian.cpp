/**
 * @file pedestrian.cpp
 * @brief Pedestrian implementation
 */

#include "pedestrian.hpp"
#include <cmath>

namespace adas
{
namespace core
{

Pedestrian::Pedestrian(
    PedestrianId id, double x, double y, double target_y, double speed) noexcept
    : id_(id), x_(x), y_(y), target_y_(target_y), speed_(speed)
{
}

void Pedestrian::startCrossing() noexcept
{
  if (state_ == PedestrianState::Waiting)
  {
    state_ = PedestrianState::Crossing;
  }
}

void Pedestrian::update(double delta_time) noexcept
{
  switch (state_)
  {
  case PedestrianState::Waiting:
    wait_timer_ += delta_time;
    if (wait_timer_ >= kMaxWaitTime)
    {
      startCrossing();
    }
    break;

  case PedestrianState::Crossing:
  {
    // Move towards target Y
    const double direction = (target_y_ > y_) ? 1.0 : -1.0;
    y_ += direction * speed_ * delta_time;

    // Check if reached target
    if ((direction > 0.0 && y_ >= target_y_) ||
        (direction < 0.0 && y_ <= target_y_))
    {
      y_ = target_y_;
      state_ = PedestrianState::Completed;
    }
    break;
  }

  case PedestrianState::Completed:
    // Pedestrian has finished crossing
    break;
  }
}

void Pedestrian::reset(double x, double y, double target_y) noexcept
{
  x_ = x;
  y_ = y;
  target_y_ = target_y;
  state_ = PedestrianState::Waiting;
  wait_timer_ = 0.0;
}

} // namespace core
} // namespace adas
