/**
 * @file gap_analyzer.cpp
 * @brief Gap analysis implementation for safe lane changing
 */

#include "gap_analyzer.hpp"
#include <algorithm>
#include <cmath>
#include <limits>

namespace adas
{
namespace perception
{

core::GapAnalysisResult
GapAnalyzer::analyze(const core::EgoVehicle& ego,
                     core::LaneId target_lane,
                     const std::vector<core::EgoVehicle>& other_vehicles,
                     double min_gap) noexcept
{
  // Initialize with infinite gaps (no vehicles detected)
  double front_gap = kInfiniteGap;
  double rear_gap = kInfiniteGap;

  const double ego_x = ego.getX();

  // Check all other vehicles
  for (const auto& vehicle : other_vehicles)
  {
    // Only consider vehicles in the target lane
    if (vehicle.getLaneId() != target_lane)
    {
      continue;
    }

    const double vehicle_x = vehicle.getX();
    const double distance = vehicle_x - ego_x;

    if (distance > 0.0)
    {
      // Vehicle is ahead
      front_gap = std::min(front_gap, distance);
    }
    else
    {
      // Vehicle is behind
      rear_gap = std::min(rear_gap, std::abs(distance));
    }
  }

  // Determine if lane change is safe
  const bool front_safe = front_gap >= min_gap;
  const bool rear_safe = rear_gap >= min_gap;
  const bool is_safe = front_safe && rear_safe;

  return core::GapAnalysisResult{is_safe, front_gap, rear_gap, min_gap, true};
}

double GapAnalyzer::calculateTimeToCollision(const core::EgoVehicle& /*ego*/,
                                             double gap,
                                             double relative_speed) noexcept
{
  // If not closing (relative speed <= 0), no collision
  if (relative_speed <= 0.0)
  {
    return std::numeric_limits<double>::infinity();
  }

  // TTC = gap / relative_speed
  return gap / relative_speed;
}

} // namespace perception
} // namespace adas
