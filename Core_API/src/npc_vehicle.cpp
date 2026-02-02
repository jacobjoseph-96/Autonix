/**
 * @file npc_vehicle.cpp
 * @brief NPC vehicle implementation
 */

#include "npc_vehicle.hpp"
#include "road_segment.hpp"
#include "traffic_light.hpp"
#include "traffic_sign.hpp"
#include <cmath>

namespace adas
{
namespace core
{

NPCVehicle::NPCVehicle(NPCId id,
                       double x,
                       LaneId lane_id,
                       double speed) noexcept
    : id_(id), x_(x), y_(0.0), lane_id_(lane_id), speed_(speed),
      target_speed_(speed)
{
}

void NPCVehicle::update(double delta_time,
                        const RoadSegment& road,
                        const std::vector<TrafficLight>& lights,
                        const std::vector<TrafficSign>& signs,
                        double vehicle_ahead_x) noexcept
{
  // Get lane center Y position
  const auto lane_boundary = road.getLaneBoundary(lane_id_);
  if (lane_boundary.has_value())
  {
    y_ = lane_boundary->centerY();
  }

  // Determine target speed based on traffic rules
  double desired_speed = target_speed_;

  // Check traffic lights
  if (shouldStopForLight(lights))
  {
    desired_speed = 0.0;
  }

  // Check stop signs
  if (shouldStopForSign(signs, delta_time))
  {
    desired_speed = 0.0;
  }

  // Check following distance
  if (vehicle_ahead_x > 0.0)
  {
    const double gap = vehicle_ahead_x - x_ - kLength;
    if (gap < kSafeFollowingDistance)
    {
      // Slow down proportionally
      const double slowdown_factor = gap / kSafeFollowingDistance;
      desired_speed = target_speed_ * slowdown_factor;
      if (gap < 5.0)
      {
        desired_speed = 0.0; // Emergency stop
      }
    }
  }

  // Smooth speed adjustment
  constexpr double kAcceleration = 3.0;
  if (speed_ < desired_speed)
  {
    speed_ = std::min(speed_ + kAcceleration * delta_time, desired_speed);
  }
  else if (speed_ > desired_speed)
  {
    speed_ = std::max(speed_ - kAcceleration * 2.0 * delta_time, desired_speed);
  }

  // Update position
  x_ += speed_ * delta_time;
}

void NPCVehicle::reset(double x) noexcept
{
  x_ = x;
  speed_ = target_speed_;
  stop_timer_ = 0.0;
  waiting_at_stop_sign_ = false;
  last_passed_sign_x_ = -1000.0; // Reset sign tracking
}

bool NPCVehicle::shouldStopForLight(
    const std::vector<TrafficLight>& lights) const noexcept
{
  for (const auto& light : lights)
  {
    // Only react to lights in our lane
    if (light.getLaneId() != lane_id_)
    {
      continue;
    }

    // Calculate distance to light (stop line)
    const double light_x = light.getPosition().x;
    const double distance_to_light = light_x - x_;

    // Only consider lights ahead and within detection range
    if (distance_to_light > 0.0 && distance_to_light < kDetectionRange)
    {
      const auto state = light.getState();
      if (state == TrafficLightState::Red || state == TrafficLightState::Yellow)
      {
        // Stop if we're close to the stop line (within stopping distance)
        // Allow vehicle to stop just before the light position
        if (distance_to_light < kSafeFollowingDistance)
        {
          return true;
        }
      }
    }
  }
  return false;
}

bool NPCVehicle::shouldStopForSign(const std::vector<TrafficSign>& signs,
                                   double delta_time) noexcept
{
  for (const auto& sign : signs)
  {
    if (sign.getType() != TrafficSignType::Stop)
    {
      continue;
    }

    // Check if sign is in our lane
    if (sign.getLaneId() != lane_id_)
    {
      continue;
    }

    const double sign_x = sign.getPosition().x;

    // Skip if we already stopped at this sign (within 1m tolerance)
    if (std::abs(sign_x - last_passed_sign_x_) < 1.0)
    {
      continue;
    }

    // Check if sign is ahead and close
    const double distance = sign_x - x_;
    if (distance > 0.0 && distance < 5.0)
    {
      if (!waiting_at_stop_sign_)
      {
        waiting_at_stop_sign_ = true;
        stop_timer_ = 0.0;
      }

      stop_timer_ += delta_time;
      if (stop_timer_ < kStopSignWaitTime)
      {
        return true;
      }
      else
      {
        // Done waiting - mark this sign as passed
        waiting_at_stop_sign_ = false;
        last_passed_sign_x_ = sign_x;
      }
    }
  }
  return false;
}

} // namespace core
} // namespace adas
