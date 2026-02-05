/**
 * @file road_segment.cpp
 * @brief Road segment implementation
 */

#include "road_segment.hpp"
#include <algorithm>
#include <cmath>

namespace adas
{
namespace core
{

RoadSegment::RoadSegment(SegmentId id, double start_x, double end_x) noexcept
    : segment_id_(id), start_x_(start_x), end_x_(end_x), lanes_()
{
}

void RoadSegment::addLane(LaneId lane_id, const LaneBoundary& boundary)
{
  // Check if lane already exists
  for (auto& lane : lanes_)
  {
    if (lane.first == lane_id)
    {
      lane.second = boundary;
      return;
    }
  }
  lanes_.emplace_back(lane_id, boundary);
}

std::optional<LaneBoundary>
RoadSegment::getLaneBoundary(LaneId lane_id) const noexcept
{
  for (const auto& lane : lanes_)
  {
    if (lane.first == lane_id)
    {
      return lane.second;
    }
  }
  return std::nullopt;
}

std::optional<LaneId> RoadSegment::findLaneAtY(double y_pos) const noexcept
{
  for (const auto& lane : lanes_)
  {
    if (lane.second.containsY(y_pos))
    {
      return lane.first;
    }
  }
  return std::nullopt;
}

bool RoadSegment::containsPosition(const Position& pos) const noexcept
{
  if (pos.x < start_x_ || pos.x > end_x_)
  {
    return false;
  }
  return findLaneAtY(pos.y).has_value();
}

std::vector<LaneId> RoadSegment::getLaneIds() const
{
  std::vector<LaneId> ids;
  ids.reserve(lanes_.size());
  for (const auto& lane : lanes_)
  {
    ids.push_back(lane.first);
  }
  return ids;
}

bool RoadSegment::areLanesAdjacent(LaneId lane1, LaneId lane2) const noexcept
{
  const auto boundary1 = getLaneBoundary(lane1);
  const auto boundary2 = getLaneBoundary(lane2);

  if (!boundary1.has_value() || !boundary2.has_value())
  {
    return false;
  }

  // Lanes are adjacent if one's edge matches the other's edge
  constexpr double kEdgeTolerance = 0.1;

  const bool edge1_matches = std::abs(boundary1->left_edge_y -
                                      boundary2->right_edge_y) < kEdgeTolerance;
  const bool edge2_matches = std::abs(boundary1->right_edge_y -
                                      boundary2->left_edge_y) < kEdgeTolerance;

  return edge1_matches || edge2_matches;
}

} // namespace core
} // namespace adas
