//! @file road_segment.hpp
//! @brief Road segment definition with lane boundaries
//!

#ifndef ADAS_CORE_ROAD_SEGMENT_HPP
#define ADAS_CORE_ROAD_SEGMENT_HPP

#include "types.hpp"
#include <utility>
#include <vector>

namespace adas
{
namespace core
{

//! @brief Lane boundary definition
//! @details Represents the left and right edges of a lane
//!
struct LaneBoundary
{
  double left_edge_y{0.0};  ///< Left edge Y coordinate
  double right_edge_y{0.0}; ///< Right edge Y coordinate

  constexpr LaneBoundary() noexcept = default;

  constexpr LaneBoundary(double left, double right) noexcept
      : left_edge_y(left), right_edge_y(right)
  {
  }

  //! @brief Check if a Y coordinate is within the lane
  //!
  [[nodiscard]] constexpr bool containsY(double y_pos) const noexcept
  {
    return (y_pos >= right_edge_y) && (y_pos <= left_edge_y);
  }

  //! @brief Get center Y position of the lane
  //!
  [[nodiscard]] constexpr double centerY() const noexcept
  {
    return (left_edge_y + right_edge_y) / 2.0;
  }
};

//! @brief Road segment containing multiple lanes
//! @details AUTOSAR compliant class with proper resource management
//!
class RoadSegment
{
public:
  //! @brief Construct a road segment
  //! @param id Unique segment identifier
  //! @param start_x Start X coordinate
  //! @param end_x End X coordinate
  //!
  explicit RoadSegment(SegmentId id,
                       double start_x = 0.0,
                       double end_x = 100.0) noexcept;

  //! @brief Add a lane to the segment
  //! @param lane_id Lane identifier
  //! @param boundary Lane boundary definition
  //!
  void addLane(LaneId lane_id, const LaneBoundary& boundary);

  //! @brief Get segment ID
  //!
  [[nodiscard]] constexpr SegmentId getId() const noexcept
  {
    return segment_id_;
  }

  //! @brief Get start X coordinate
  //!
  [[nodiscard]] constexpr double getStartX() const noexcept
  {
    return start_x_;
  }

  //! @brief Get end X coordinate
  //!
  [[nodiscard]] constexpr double getEndX() const noexcept
  {
    return end_x_;
  }

  //! @brief Get lane boundary by ID
  //! @param lane_id Lane identifier
  //! @return Optional lane boundary
  //!
  [[nodiscard]] std::optional<LaneBoundary>
  getLaneBoundary(LaneId lane_id) const noexcept;

  //! @brief Find lane ID for a given Y position
  //! @param y_pos Y coordinate
  //! @return Optional lane ID if found
  //!
  [[nodiscard]] std::optional<LaneId> findLaneAtY(double y_pos) const noexcept;

  //! @brief Check if position is within segment bounds
  //!
  [[nodiscard]] bool containsPosition(const Position& pos) const noexcept;

  //! @brief Get all lane IDs in this segment
  //!
  [[nodiscard]] std::vector<LaneId> getLaneIds() const;

  //! @brief Check if two lanes are adjacent
  //!
  [[nodiscard]] bool areLanesAdjacent(LaneId lane1,
                                      LaneId lane2) const noexcept;

private:
  SegmentId segment_id_;
  double start_x_;
  double end_x_;
  std::vector<std::pair<LaneId, LaneBoundary>> lanes_;
};

} // namespace core
} // namespace adas

#endif // ADAS_CORE_ROAD_SEGMENT_HPP
