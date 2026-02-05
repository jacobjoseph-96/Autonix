//! @file adaptive_cruise_controller.cpp
//! @brief Adaptive Cruise Control implementation
//!

#include "adaptive_cruise_controller.hpp"
#include <algorithm>
#include <cmath>

namespace adas
{
namespace core
{

AdaptiveCruiseController::AdaptiveCruiseController() noexcept
    : is_enabled_(false), mode_(ACCMode::Comfort),
      config_(ACCConfig::forMode(ACCMode::Comfort)),
      target_speed_(kDefaultTargetSpeed)
{
}

void AdaptiveCruiseController::enable() noexcept
{
  is_enabled_ = true;
}

void AdaptiveCruiseController::disable() noexcept
{
  is_enabled_ = false;
}

void AdaptiveCruiseController::setMode(ACCMode mode) noexcept
{
  mode_ = mode;
  config_ = ACCConfig::forMode(mode);
}

void AdaptiveCruiseController::setTargetSpeed(double speed) noexcept
{
  target_speed_ = std::clamp(speed, kMinTargetSpeed, kMaxTargetSpeed);
}

void AdaptiveCruiseController::setTargetSpeedKmh(double speed_kmh) noexcept
{
  // Convert km/h to m/s: divide by 3.6
  setTargetSpeed(speed_kmh / 3.6);
}

double AdaptiveCruiseController::computeTargetSpeed(
    const EgoVehicle& ego,
    std::optional<Coordinate> lead_position,
    double lead_speed) const noexcept
{
  // If ACC is disabled, return current speed (no intervention)
  if (!is_enabled_)
  {
    return ego.getSpeed();
  }

  // If no lead vehicle detected, cruise at target speed
  if (!lead_position.has_value())
  {
    return target_speed_;
  }

  // Calculate distance to lead vehicle
  const double ego_x = ego.getX();
  const double lead_x = lead_position->x;
  const double distance = lead_x - ego_x;

  // Emergency stop if lead is behind or collision imminent
  if (distance <= 0.0)
  {
    return 0.0;
  }

  // Calculate required safety gap based on current speed and configuration
  // Gap = speed * time_gap + minimum_buffer
  const double current_speed = ego.getSpeed();
  const double safe_gap =
      std::max(current_speed * config_.time_gap, config_.min_following_dist);

  if (distance > safe_gap)
  {
    // Safe to cruise at target speed
    return target_speed_;
  }
  else
  {
    // Need to match or reduce to lead vehicle speed
    // If significantly closer than safe gap, slow down more aggressively
    const double gap_ratio = distance / safe_gap;

    if (gap_ratio < 0.5)
    {
      // Very close - slow down significantly below lead speed
      return std::min(lead_speed * 0.8, target_speed_);
    }
    else
    {
      // Moderately close - match lead speed
      return std::min(lead_speed, target_speed_);
    }
  }
}

double AdaptiveCruiseController::applyAccelerationLimits(
    double current_speed,
    double desired_speed,
    double delta_time) const noexcept
{
  const double speed_diff = desired_speed - current_speed;

  if (speed_diff > 0.0)
  {
    // Accelerating
    const double max_speed_increase = config_.max_acceleration * delta_time;
    return current_speed + std::min(speed_diff, max_speed_increase);
  }
  else if (speed_diff < 0.0)
  {
    // Decelerating
    const double max_speed_decrease = config_.max_deceleration * delta_time;
    return current_speed - std::min(-speed_diff, max_speed_decrease);
  }

  return current_speed;
}

void AdaptiveCruiseController::update(EgoVehicle& ego,
                                      std::optional<Coordinate> lead_position,
                                      double lead_speed,
                                      double delta_time) noexcept
{
  if (!is_enabled_)
  {
    return; // ACC not active, don't modify vehicle speed
  }

  // Compute desired target speed based on lead vehicle
  const double desired_speed =
      computeTargetSpeed(ego, lead_position, lead_speed);

  // Apply acceleration/deceleration limits for smooth control
  const double new_speed =
      applyAccelerationLimits(ego.getSpeed(), desired_speed, delta_time);

  // Update vehicle speed
  ego.setSpeed(new_speed);
}

} // namespace core
} // namespace adas
