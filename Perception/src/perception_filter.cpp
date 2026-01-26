/**
 * @file perception_filter.cpp
 * @brief Perception filter implementation
 */

#include "perception_filter.hpp"
#include <algorithm>

namespace adas
{
namespace perception
{

PerceptionFilter::PerceptionFilter(double fov_radians,
                                   double max_range) noexcept
    : fov_radians_(fov_radians), max_range_(max_range),
      half_fov_(fov_radians / 2.0)
{
}

std::vector<DetectionResult>
PerceptionFilter::filter(const core::EgoVehicle& ego,
                         const std::vector<core::TrafficSign>& signs,
                         const core::RoadSegment& road) const
{
  std::vector<DetectionResult> results;
  results.reserve(signs.size());

  for (const auto& sign : signs)
  {
    if (!sign.isValid())
    {
      continue;
    }

    const double distance = SafeDistanceCalculator::calculate(
        ego.getPosition(), sign.getPosition());

    // Skip signs outside max range
    if (distance > max_range_)
    {
      continue;
    }

    const bool in_fov = isInFieldOfView(ego, sign.getPosition());
    const bool is_relevant =
        isLaneRelevant(ego.getLaneId(), sign.getLaneId(), road);

    // Only include signs that are in FoV
    if (in_fov)
    {
      results.emplace_back(sign, distance, is_relevant, in_fov);
    }
  }

  // Sort by distance (closest first)
  std::sort(results.begin(),
            results.end(),
            [](const DetectionResult& a, const DetectionResult& b)
            { return a.distance < b.distance; });

  return results;
}

bool PerceptionFilter::isInFieldOfView(
    const core::EgoVehicle& ego,
    const core::Position& sign_position) const noexcept
{
  // Calculate bearing from ego to sign
  const double bearing = calculateBearing(ego.getPosition(), sign_position);

  // Calculate relative angle (difference from heading)
  const double relative_angle = normalizeAngle(bearing - ego.getHeading());

  // Check if within FoV cone
  return std::abs(relative_angle) <= half_fov_;
}

bool PerceptionFilter::isLaneRelevant(
    core::LaneId ego_lane_id,
    core::LaneId sign_lane_id,
    const core::RoadSegment& road) const noexcept
{
  // Same lane is always relevant
  if (ego_lane_id == sign_lane_id)
  {
    return true;
  }

  // Check if lanes are adjacent
  return road.areLanesAdjacent(ego_lane_id, sign_lane_id);
}

double PerceptionFilter::calculateBearing(const core::Coordinate& from,
                                          const core::Coordinate& to) noexcept
{
  const double dx = to.x - from.x;
  const double dy = to.y - from.y;

  return std::atan2(dy, dx);
}

double PerceptionFilter::normalizeAngle(double angle) noexcept
{
  while (angle > M_PI)
  {
    angle -= 2.0 * M_PI;
  }
  while (angle < -M_PI)
  {
    angle += 2.0 * M_PI;
  }
  return angle;
}

} // namespace perception
} // namespace adas
