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

  //! @brief Vehicle dimensions
  //!
  static constexpr double kLength = 4.5; ///< Vehicle length in meters
  static constexpr double kWidth = 2.0;  ///< Vehicle width in meters

private:
  Coordinate position_;
  LaneId current_lane_id_;
  double speed_{0.0};

  static constexpr double kMaxSpeed = 50.0; ///< Max speed in m/s (~180 km/h)
  static constexpr double kMinSpeed = 0.0;
};

} // namespace core
} // namespace adas

#endif // ADAS_CORE_EGO_VEHICLE_HPP
