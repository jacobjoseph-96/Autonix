//! @file gap_analyzer.hpp
//! @brief Gap analysis for safe lane changing
//!

#ifndef ADAS_PERCEPTION_GAP_ANALYZER_HPP
#define ADAS_PERCEPTION_GAP_ANALYZER_HPP

#include "ego_vehicle.hpp"
#include "types.hpp"
#include <vector>

namespace adas
{
namespace perception
{

//! @brief Analyzes gaps in adjacent lanes for safe lane changing
//! @details Calculates front and rear gaps to other vehicles in target lane
//!
class GapAnalyzer
{
public:
  //! @brief Default minimum safe gap in meters
  //!
  static constexpr double kDefaultMinGap = 10.0;

  //! @brief Large value representing infinite gap (no vehicle)
  //!
  static constexpr double kInfiniteGap = 1000.0;

  //! @brief Analyze gap in target lane
  //! @param ego Ego vehicle state
  //! @param target_lane Target lane ID
  //! @param other_vehicles Other vehicles on the road
  //! @param min_gap Minimum safe gap required
  //! @return Gap analysis result
  //!
  [[nodiscard]] static core::GapAnalysisResult
  analyze(const core::EgoVehicle& ego,
          core::LaneId target_lane,
          const std::vector<core::EgoVehicle>& other_vehicles,
          double min_gap = kDefaultMinGap) noexcept;

  //! @brief Check if gap analysis result indicates safe lane change
  //! @param result Gap analysis result
  //! @return true if lane change is safe
  //!
  [[nodiscard]] static constexpr bool
  isLaneChangeSafe(const core::GapAnalysisResult& result) noexcept
  {
    return result.is_safe && result.target_lane_exists;
  }

  //! @brief Calculate time to collision if lane change executed
  //! @param ego Ego vehicle
  //! @param gap Distance gap
  //! @param relative_speed Relative speed (positive = closing)
  //! @return Time to collision in seconds (infinity if not closing)
  //!
  [[nodiscard]] static double calculateTimeToCollision(
      const core::EgoVehicle& ego, double gap, double relative_speed) noexcept;
};

} // namespace perception
} // namespace adas

#endif // ADAS_PERCEPTION_GAP_ANALYZER_HPP
