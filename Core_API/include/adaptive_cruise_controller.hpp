//! @file adaptive_cruise_controller.hpp
//! @brief Adaptive Cruise Control (ACC) controller
//!

#ifndef ADAS_CORE_ADAPTIVE_CRUISE_CONTROLLER_HPP
#define ADAS_CORE_ADAPTIVE_CRUISE_CONTROLLER_HPP

#include "ego_vehicle.hpp"
#include "types.hpp"
#include <optional>

namespace adas
{
namespace core
{

//! @brief Adaptive Cruise Control controller
//! @details Manages automatic speed control based on lead vehicle and user
//!          settings. Supports three driving modes: Eco, Comfort, Sport.
//!
class AdaptiveCruiseController
{
public:
  //! @brief Minimum target speed in m/s (~30 km/h)
  //!
  static constexpr double kMinTargetSpeed = 8.33;

  //! @brief Maximum target speed in m/s (~130 km/h)
  //!
  static constexpr double kMaxTargetSpeed = 36.11;

  //! @brief Default target speed in m/s (~80 km/h)
  //!
  static constexpr double kDefaultTargetSpeed = 22.22;

  //! @brief Construct ACC controller with default settings
  //!
  AdaptiveCruiseController() noexcept;

  //! @brief Enable ACC
  //!
  void enable() noexcept;

  //! @brief Disable ACC
  //!
  void disable() noexcept;

  //! @brief Check if ACC is enabled
  //!
  [[nodiscard]] constexpr bool isEnabled() const noexcept
  {
    return is_enabled_;
  }

  //! @brief Set ACC mode
  //! @param mode New driving mode (Eco, Comfort, Sport)
  //!
  void setMode(ACCMode mode) noexcept;

  //! @brief Get current ACC mode
  //!
  [[nodiscard]] constexpr ACCMode getMode() const noexcept
  {
    return mode_;
  }

  //! @brief Get current configuration
  //!
  [[nodiscard]] constexpr const ACCConfig& getConfig() const noexcept
  {
    return config_;
  }

  //! @brief Set target speed
  //! @param speed Target speed in m/s (clamped to valid range)
  //!
  void setTargetSpeed(double speed) noexcept;

  //! @brief Set target speed from km/h
  //! @param speed_kmh Target speed in km/h
  //!
  void setTargetSpeedKmh(double speed_kmh) noexcept;

  //! @brief Get target speed in m/s
  //!
  [[nodiscard]] constexpr double getTargetSpeed() const noexcept
  {
    return target_speed_;
  }

  //! @brief Get target speed in km/h
  //!
  [[nodiscard]] constexpr double getTargetSpeedKmh() const noexcept
  {
    return target_speed_ * 3.6;
  }

  //! @brief Compute desired speed based on lead vehicle
  //! @param ego Ego vehicle state
  //! @param lead_position Position of lead vehicle (nullopt if no lead)
  //! @param lead_speed Speed of lead vehicle (ignored if no lead)
  //! @return Computed target speed in m/s
  //!
  [[nodiscard]] double
  computeTargetSpeed(const EgoVehicle& ego,
                     std::optional<Coordinate> lead_position,
                     double lead_speed = 0.0) const noexcept;

  //! @brief Update ego vehicle speed based on ACC logic
  //! @param ego Ego vehicle to control
  //! @param lead_position Position of lead vehicle (nullopt if no lead)
  //! @param lead_speed Speed of lead vehicle
  //! @param delta_time Time step in seconds
  //!
  void update(EgoVehicle& ego,
              std::optional<Coordinate> lead_position,
              double lead_speed,
              double delta_time) noexcept;

private:
  bool is_enabled_{false};
  ACCMode mode_{ACCMode::Comfort};
  ACCConfig config_{ACCConfig::forMode(ACCMode::Comfort)};
  double target_speed_{kDefaultTargetSpeed};

  //! @brief Apply acceleration limits to speed change
  //! @param current_speed Current speed
  //! @param desired_speed Desired target speed
  //! @param delta_time Time step
  //! @return Speed after applying acceleration limits
  //!
  [[nodiscard]] double
  applyAccelerationLimits(double current_speed,
                          double desired_speed,
                          double delta_time) const noexcept;
};

} // namespace core
} // namespace adas

#endif // ADAS_CORE_ADAPTIVE_CRUISE_CONTROLLER_HPP
