//! @file ego_vehicle.hpp
//! @brief Ego vehicle (self-driving car) representation
//!

#ifndef ADAS_CORE_EGO_VEHICLE_HPP
#define ADAS_CORE_EGO_VEHICLE_HPP

#include "types.hpp"
#include <optional>

namespace adas
{
namespace core
{

//! @brief Ego vehicle state and properties
//! @details Represents the self-driving car with position, heading, and lane
//! information
//!
class EgoVehicle
{
public:
  //! @brief Construct ego vehicle at initial position
  //! @param position Initial position (x, y, heading)
  //! @param lane_id Initial lane ID
  //!
  explicit EgoVehicle(const Coordinate& position = Coordinate{},
                      LaneId lane_id = 1U) noexcept;

  //! @brief Get current position
  //!
  [[nodiscard]] constexpr const Coordinate& getPosition() const noexcept
  {
    return position_;
  }

  //! @brief Get X coordinate
  //!
  [[nodiscard]] constexpr double getX() const noexcept
  {
    return position_.x;
  }

  //! @brief Get Y coordinate
  //!
  [[nodiscard]] constexpr double getY() const noexcept
  {
    return position_.y;
  }

  //! @brief Get heading angle in radians
  //!
  [[nodiscard]] constexpr double getHeading() const noexcept
  {
    return position_.theta;
  }

  //! @brief Get current lane ID
  //!
  [[nodiscard]] constexpr LaneId getLaneId() const noexcept
  {
    return current_lane_id_;
  }

  //! @brief Get vehicle speed in m/s
  //!
  [[nodiscard]] constexpr double getSpeed() const noexcept
  {
    return speed_;
  }

  //! @brief Set vehicle position
  //! @param position New position
  //!
  void setPosition(const Coordinate& position) noexcept;

  //! @brief Set current lane ID
  //! @param lane_id New lane ID
  //!
  void setLaneId(LaneId lane_id) noexcept;

  //! @brief Set vehicle speed
  //! @param speed Speed in m/s (clamped to valid range)
  //!
  void setSpeed(double speed) noexcept;

  //! @brief Update position based on speed and heading
  //! @param delta_time Time step in seconds
  //!
  void update(double delta_time) noexcept;

  //! @brief Move vehicle forward by distance
  //! @param distance Distance in meters
  //!
  void moveForward(double distance) noexcept;

  //! @brief Rotate vehicle by angle
  //! @param angle Angle in radians (positive = counter-clockwise)
  //!
  void rotate(double angle) noexcept;

  //! @brief Get turn signal state
  //!
  [[nodiscard]] constexpr TurnSignalState getTurnSignal() const noexcept
  {
    return turn_signal_;
  }

  //! @brief Check if lane change is in progress
  //!
  [[nodiscard]] constexpr bool isChangingLane() const noexcept
  {
    return is_changing_lane_;
  }

  //! @brief Get lane change progress (0.0 to 1.0)
  //!
  [[nodiscard]] constexpr double getLaneChangeProgress() const noexcept
  {
    return lane_change_progress_;
  }

  //! @brief Get target lane ID during lane change
  //!
  [[nodiscard]] constexpr LaneId getTargetLaneId() const noexcept
  {
    return target_lane_id_;
  }

  //! @brief Get lane change direction
  //!
  [[nodiscard]] constexpr LaneChangeDirection
  getLaneChangeDirection() const noexcept
  {
    return lane_change_direction_;
  }

  //! @brief Set turn signal state
  //! @param state New turn signal state
  //!
  void setTurnSignal(TurnSignalState state) noexcept;

  //! @brief Start a lane change maneuver
  //! @param direction Direction of lane change
  //! @param target_lane Target lane ID
  //! @param start_y Starting Y position
  //! @param target_y Target Y position
  //!
  void startLaneChange(LaneChangeDirection direction,
                       LaneId target_lane,
                       double start_y,
                       double target_y) noexcept;

  //! @brief Update lane change progress
  //! @param delta_time Time step in seconds
  //! @param duration Total lane change duration
  //! @return true if lane change completed this frame
  //!
  bool updateLaneChange(double delta_time, double duration) noexcept;

  //! @brief Cancel lane change in progress
  //!
  void cancelLaneChange() noexcept;

  //! @brief Complete lane change (set final lane ID)
  //!
  void completeLaneChange() noexcept;

  //! @brief Vehicle dimensions
  //!
  static constexpr double kLength = 4.5; ///< Vehicle length in meters
  static constexpr double kWidth = 2.0;  ///< Vehicle width in meters

private:
  Coordinate position_;
  LaneId current_lane_id_;
  double speed_{0.0};

  // Lane change state
  TurnSignalState turn_signal_{TurnSignalState::Off};
  LaneChangeDirection lane_change_direction_{LaneChangeDirection::None};
  LaneId target_lane_id_{0U};
  double lane_change_progress_{0.0};
  double lane_change_start_y_{0.0};
  double lane_change_target_y_{0.0};
  bool is_changing_lane_{false};

  static constexpr double kMaxSpeed = 50.0; ///< Max speed in m/s (~180 km/h)
  static constexpr double kMinSpeed = 0.0;
};

} // namespace core
} // namespace adas

#endif // ADAS_CORE_EGO_VEHICLE_HPP
