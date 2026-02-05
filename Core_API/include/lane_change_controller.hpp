//! @file lane_change_controller.hpp
//! @brief Lane change controller for managing lane change maneuvers
//!

#ifndef ADAS_CORE_LANE_CHANGE_CONTROLLER_HPP
#define ADAS_CORE_LANE_CHANGE_CONTROLLER_HPP

#include "ego_vehicle.hpp"
#include "road_segment.hpp"
#include "types.hpp"

namespace adas
{
namespace core
{

//! @brief State of the lane change controller
//!
enum class LaneChangeState : std::uint8_t
{
  Idle = 0U,      ///< No lane change in progress
  Signaling = 1U, ///< Turn signal on, waiting before maneuver
  Executing = 2U, ///< Lane change in progress
  Completing = 3U ///< Finishing lane change
};

//! @brief Controller for managing lane change maneuvers
//! @details Handles turn signal timing, gap analysis integration, and smooth
//!          lateral movement during lane changes
//!
class LaneChangeController
{
public:
  //! @brief Duration of lane change maneuver in seconds
  //!
  static constexpr double kLaneChangeDuration = 2.0;

  //! @brief Time to signal before starting lane change
  //!
  static constexpr double kSignalLeadTime = 0.5;

  //! @brief Minimum speed required for lane change (m/s)
  //!
  static constexpr double kMinSpeedForLaneChange = 2.0;

  //! @brief Construct lane change controller
  //!
  LaneChangeController() noexcept = default;

  //! @brief Request a lane change
  //! @param vehicle Ego vehicle (mutable for turn signal)
  //! @param direction Direction of lane change
  //! @param road Road segment for lane validation
  //! @param gap_result Result from gap analysis
  //! @return true if lane change request accepted
  //!
  [[nodiscard]] bool
  requestLaneChange(EgoVehicle& vehicle,
                    LaneChangeDirection direction,
                    const RoadSegment& road,
                    const GapAnalysisResult& gap_result) noexcept;

  //! @brief Update lane change in progress
  //! @param vehicle Ego vehicle to update
  //! @param road Road segment for lane boundaries
  //! @param delta_time Time step in seconds
  //!
  void update(EgoVehicle& vehicle,
              const RoadSegment& road,
              double delta_time) noexcept;

  //! @brief Cancel lane change in progress
  //! @param vehicle Ego vehicle
  //!
  void cancel(EgoVehicle& vehicle) noexcept;

  //! @brief Check if lane change is active
  //!
  [[nodiscard]] constexpr bool isActive() const noexcept
  {
    return state_ != LaneChangeState::Idle;
  }

  //! @brief Get current state
  //!
  [[nodiscard]] constexpr LaneChangeState getState() const noexcept
  {
    return state_;
  }

  //! @brief Get target lane ID
  //!
  [[nodiscard]] constexpr LaneId getTargetLane() const noexcept
  {
    return target_lane_id_;
  }

private:
  LaneChangeState state_{LaneChangeState::Idle};
  LaneChangeDirection direction_{LaneChangeDirection::None};
  LaneId target_lane_id_{0U};
  double timer_{0.0};
  double start_y_{0.0};
  double target_y_{0.0};

  //! @brief Calculate target lane ID based on direction
  //!
  [[nodiscard]] static LaneId
  calculateTargetLane(LaneId current_lane,
                      LaneChangeDirection direction) noexcept;
};

} // namespace core
} // namespace adas

#endif // ADAS_CORE_LANE_CHANGE_CONTROLLER_HPP
