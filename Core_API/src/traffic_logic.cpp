#include "traffic_logic.hpp"
#include <cmath>

namespace adas
{
namespace core
{

bool TrafficLogic::shouldStopForSign(const EgoVehicle& vehicle,
                                     const std::vector<TrafficSign>& signs,
                                     StopSignState& state,
                                     double dt)
{
  const double vehicle_x = vehicle.getX();
  const LaneId vehicle_lane = vehicle.getLaneId();

  // If currently waiting at a stop sign, decrement timer
  if (state.was_stopped && state.wait_timer > 0.0)
  {
    state.wait_timer -= dt;
    if (state.wait_timer <= 0.0)
    {
      // Done waiting, can proceed
      state.was_stopped = false;
      state.processed_signs.insert(state.current_sign_id);
      state.current_sign_id = 0U;
      return false;
    }
    return true; // Still waiting
  }

  for (const auto& sign : signs)
  {
    // Only check stop signs in vehicle's lane
    if (sign.getType() != TrafficSignType::Stop ||
        sign.getLaneId() != vehicle_lane)
    {
      continue;
    }

    // Skip if we've already processed this stop sign
    if (state.processed_signs.count(sign.getId()) > 0)
    {
      continue;
    }

    // Skip if we've already processed this stop sign (redundant check with
    // current logic but safe)
    if (sign.getId() == state.current_sign_id && !state.was_stopped)
    {
      continue;
    }

    const double sign_x = sign.getPosition().x;
    const double distance = sign_x - vehicle_x;

    // Sign is ahead and within stop range
    if (distance > 0.0 && distance <= kStopLineBuffer &&
        distance < kDetectionRange)
    {
      // Start waiting at this stop sign
      if (!state.was_stopped)
      {
        state.was_stopped = true;
        state.wait_timer = kStopSignWaitTime;
        state.current_sign_id = sign.getId();
      }
      return true;
    }
  }

  return false;
}

bool TrafficLogic::shouldStopForLight(const EgoVehicle& vehicle,
                                      const std::vector<TrafficLight>& lights)
{
  const double vehicle_x = vehicle.getX();
  const LaneId vehicle_lane = vehicle.getLaneId();

  for (const auto& light : lights)
  {
    // Check if light is in vehicle's lane
    if (light.getLaneId() != vehicle_lane)
    {
      continue;
    }

    const double light_x = light.getX();
    const double distance = light_x - vehicle_x;

    // Light is ahead and within detection range
    if (distance > 0.0 && distance < kDetectionRange)
    {
      // Should stop if red or yellow and approaching or at stop line
      if (light.shouldStop() && distance <= kStopLineBuffer)
      {
        return true;
      }
    }
  }

  return false;
}

bool TrafficLogic::isPositionValid(
    double candidate_x, const std::vector<TrafficLight>& existing_lights)
{
  for (const auto& light : existing_lights)
  {
    if (std::abs(light.getX() - candidate_x) < kMinLightDistance)
    {
      return false;
    }
  }
  return true;
}

double TrafficLogic::computeACCSpeed(const EgoVehicle& ego,
                                     const LeadVehicle& lead,
                                     double set_speed,
                                     double time_gap)
{
  const double ego_x = ego.getX();
  const double lead_x = lead.getPosition().x;

  // Basic distance check (assuming lead is ahead in X)
  const double distance = lead_x - ego_x;

  // If lead is behind or colliding, emergency stop (or return 0)
  if (distance <= 0.0)
  {
    return 0.0;
  }

  // Calculate required safety gap:
  // Gap = current_speed * time_gap + minimum_buffer (e.g. 5m)
  // Using set_speed or current speed? Usually current speed.
  const double current_speed = ego.getSpeed();
  const double safe_gap =
      std::max(current_speed * time_gap, 5.0); // Min 5m buffer

  if (distance > safe_gap)
  {
    // Safe to cruise at set speed
    return set_speed;
  }
  else
  {
    // Need to match lead vehicle speed
    // Ideally we would decelerate, but for this simulation step we return
    // target speed If we are significantly closer than safe gap, we might want
    // to slow down MORE than lead to rebuild the gap. Simple logic: return lead
    // speed. Improvement: return min(lead_speed, set_speed)
    return std::min(lead.getSpeed(), set_speed);
  }
}

} // namespace core
} // namespace adas
