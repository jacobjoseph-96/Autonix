//! @file npc_vehicle.hpp
//! @brief NPC (AI-controlled) vehicle for traffic simulation
//!

#ifndef ADAS_CORE_NPC_VEHICLE_HPP
#define ADAS_CORE_NPC_VEHICLE_HPP

#include "traffic_light.hpp"
#include "traffic_sign.hpp"
#include "types.hpp"
#include <cstdint>
#include <vector>

namespace adas
{
namespace core
{

// Forward declaration
class RoadSegment;

//! @brief NPC vehicle with AI behavior
//! @details AI-controlled vehicle that follows lanes and obeys traffic rules
//!
class NPCVehicle
{
public:
  //! @brief Unique identifier for NPC vehicles
  //!
  using NPCId = std::uint32_t;

  //! @brief Construct NPC at position
  //! @param id Unique NPC identifier
  //! @param x Initial X position
  //! @param lane_id Lane to drive in
  //! @param speed Constant driving speed
  //!
  NPCVehicle(NPCId id, double x, LaneId lane_id, double speed) noexcept;

  //! @brief Get NPC ID
  //!
  [[nodiscard]] constexpr NPCId getId() const noexcept
  {
    return id_;
  }

  //! @brief Get X position
  //!
  [[nodiscard]] constexpr double getX() const noexcept
  {
    return x_;
  }

  //! @brief Get Y position
  //!
  [[nodiscard]] constexpr double getY() const noexcept
  {
    return y_;
  }

  //! @brief Get current lane ID
  //!
  [[nodiscard]] constexpr LaneId getLaneId() const noexcept
  {
    return lane_id_;
  }

  //! @brief Get current speed
  //!
  [[nodiscard]] constexpr double getSpeed() const noexcept
  {
    return speed_;
  }

  //! @brief Get target speed
  //!
  [[nodiscard]] constexpr double getTargetSpeed() const noexcept
  {
    return target_speed_;
  }

  //! @brief Check if NPC is stopped
  //!
  [[nodiscard]] constexpr bool isStopped() const noexcept
  {
    return speed_ < 0.1;
  }

  //! @brief Set Y position (for lane center)
  //!
  void setY(double y) noexcept
  {
    y_ = y;
  }

  //! @brief Update NPC position and behavior
  //! @param delta_time Time step in seconds
  //! @param road Road segment for lane info
  //! @param lights Traffic lights to obey
  //! @param signs Traffic signs to obey
  //! @param vehicle_ahead_x X position of vehicle ahead (-1 if none)
  //!
  void update(double delta_time,
              const RoadSegment& road,
              const std::vector<TrafficLight>& lights,
              const std::vector<TrafficSign>& signs,
              double vehicle_ahead_x) noexcept;

  //! @brief Reset NPC to initial state
  //!
  void reset(double x) noexcept;

  //! @brief Vehicle dimensions
  //!
  static constexpr double kLength = 4.5;
  static constexpr double kWidth = 2.0;

private:
  NPCId id_;
  double x_;
  double y_;
  LaneId lane_id_;
  double speed_;
  double target_speed_;

  // Behavior state
  double stop_timer_{0.0};
  bool waiting_at_stop_sign_{false};
  double last_passed_sign_x_{-1000.0}; // X pos of last stop sign we stopped at

  //! @brief Check if should stop for traffic light
  //!
  [[nodiscard]] bool
  shouldStopForLight(const std::vector<TrafficLight>& lights) const noexcept;

  //! @brief Check if should stop for stop sign
  //!
  [[nodiscard]] bool shouldStopForSign(const std::vector<TrafficSign>& signs,
                                       double delta_time) noexcept;

  //! @brief Safe following distance in meters
  //!
  static constexpr double kSafeFollowingDistance = 15.0;

  //! @brief Stop sign wait time in seconds
  //!
  static constexpr double kStopSignWaitTime = 0.3;

  //! @brief Detection range for traffic elements
  //!
  static constexpr double kDetectionRange = 30.0;
};

} // namespace core
} // namespace adas

#endif // ADAS_CORE_NPC_VEHICLE_HPP
