//! @file traffic_light.hpp
//! @brief Traffic light definition with state management
//!

#ifndef ADAS_CORE_TRAFFIC_LIGHT_HPP
#define ADAS_CORE_TRAFFIC_LIGHT_HPP

#include "types.hpp"

namespace adas
{
namespace core
{

//! @brief Traffic light entity with cycling state
//! @details Represents a traffic light at a specific position that cycles
//! through states
//!
class TrafficLight
{
public:
  //! @brief Factory function for safe construction
  //! @param id Light identifier
  //! @param position Light position
  //! @param lane_id Associated lane ID
  //! @param initial_state Initial light state
  //! @param cycle_duration Duration of full cycle in seconds
  //! @return Constructed traffic light
  //!
  [[nodiscard]] static TrafficLight
  create(TrafficLightId id,
         const Position& position,
         LaneId lane_id,
         TrafficLightState initial_state = TrafficLightState::Red,
         double cycle_duration = 10.0) noexcept;

  // Default constructor for container compatibility
  TrafficLight() noexcept = default;

  //! @brief Update traffic light state based on elapsed time
  //! @param delta_time Time elapsed since last update in seconds
  //!
  void update(double delta_time) noexcept;

  //! @brief Get light ID
  //!
  [[nodiscard]] constexpr TrafficLightId getId() const noexcept
  {
    return id_;
  }

  //! @brief Get current state
  //!
  [[nodiscard]] constexpr TrafficLightState getState() const noexcept
  {
    return state_;
  }

  //! @brief Get light position
  //!
  [[nodiscard]] constexpr const Position& getPosition() const noexcept
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

  //! @brief Get associated lane ID
  //!
  [[nodiscard]] constexpr LaneId getLaneId() const noexcept
  {
    return lane_id_;
  }

  //! @brief Check if light is red
  //!
  [[nodiscard]] constexpr bool isRed() const noexcept
  {
    return state_ == TrafficLightState::Red;
  }

  //! @brief Check if light is yellow
  //!
  [[nodiscard]] constexpr bool isYellow() const noexcept
  {
    return state_ == TrafficLightState::Yellow;
  }

  //! @brief Check if light is green
  //!
  [[nodiscard]] constexpr bool isGreen() const noexcept
  {
    return state_ == TrafficLightState::Green;
  }

  //! @brief Check if vehicle should stop (red or yellow)
  //!
  [[nodiscard]] constexpr bool shouldStop() const noexcept
  {
    return state_ == TrafficLightState::Red ||
           state_ == TrafficLightState::Yellow;
  }

  //! @brief Check if light is valid (has been properly constructed)
  //!
  [[nodiscard]] constexpr bool isValid() const noexcept
  {
    return id_ != 0U;
  }

  //! @brief Get time remaining in current state
  //!
  [[nodiscard]] constexpr double getTimeRemaining() const noexcept
  {
    return state_duration_ - state_timer_;
  }

private:
  TrafficLight(TrafficLightId id,
               const Position& position,
               LaneId lane_id,
               TrafficLightState initial_state,
               double cycle_duration) noexcept;

  void advanceState() noexcept;

  TrafficLightId id_{0U};
  Position position_{};
  LaneId lane_id_{0U};
  TrafficLightState state_{TrafficLightState::Red};

  double cycle_duration_{10.0};
  double state_timer_{0.0};
  double state_duration_{0.0};

  // State durations as fractions of cycle
  static constexpr double kRedFraction = 0.45;
  static constexpr double kYellowFraction = 0.10;
  static constexpr double kGreenFraction = 0.45;
};

//! @brief Convert traffic light state to string representation
//! @param state The traffic light state
//! @return String representation of the state
//!
[[nodiscard]] inline const char *
trafficLightStateToString(TrafficLightState state) noexcept
{
  switch (state)
  {
  case TrafficLightState::Red:
    return "Red";
  case TrafficLightState::Yellow:
    return "Yellow";
  case TrafficLightState::Green:
    return "Green";
  default:
    return "Unknown";
  }
}

} // namespace core
} // namespace adas

#endif // ADAS_CORE_TRAFFIC_LIGHT_HPP
