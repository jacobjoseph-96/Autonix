//! @file pedestrian.hpp
//! @brief Pedestrian entity for crossing simulation
//!

#ifndef ADAS_CORE_PEDESTRIAN_HPP
#define ADAS_CORE_PEDESTRIAN_HPP

#include "types.hpp"
#include <cstdint>

namespace adas
{
namespace core
{

//! @brief Pedestrian crossing state
enum class PedestrianState : std::uint8_t
{
  Waiting,  //!< Waiting to cross
  Crossing, //!< Currently crossing
  Completed //!< Finished crossing
};

//! @brief Pedestrian entity with crossing behavior
class Pedestrian
{
public:
  using PedestrianId = std::uint32_t;

  //! @brief Default constructor
  Pedestrian() noexcept = default;

  //! @brief Construct pedestrian at position
  //! @param id Unique identifier
  //! @param x X position (along road)
  //! @param y Y position (starting side of road)
  //! @param target_y Target Y (other side of road)
  //! @param speed Walking speed m/s
  //!
  Pedestrian(PedestrianId id,
             double x,
             double y,
             double target_y,
             double speed = 1.2) noexcept;

  //! @brief Get pedestrian ID
  [[nodiscard]] constexpr PedestrianId getId() const noexcept
  {
    return id_;
  }

  //! @brief Get X position
  [[nodiscard]] constexpr double getX() const noexcept
  {
    return x_;
  }

  //! @brief Get Y position
  [[nodiscard]] constexpr double getY() const noexcept
  {
    return y_;
  }

  //! @brief Get walking speed
  [[nodiscard]] constexpr double getSpeed() const noexcept
  {
    return speed_;
  }

  //! @brief Get current state
  [[nodiscard]] constexpr PedestrianState getState() const noexcept
  {
    return state_;
  }

  //! @brief Check if currently crossing
  [[nodiscard]] constexpr bool isCrossing() const noexcept
  {
    return state_ == PedestrianState::Crossing;
  }

  //! @brief Start crossing
  void startCrossing() noexcept;

  //! @brief Update pedestrian position
  //! @param delta_time Time step in seconds
  void update(double delta_time) noexcept;

  //! @brief Reset pedestrian
  void reset(double x, double y, double target_y) noexcept;

  //! @brief Pedestrian dimensions (approximate)
  static constexpr double kWidth = 0.5;
  static constexpr double kHeight = 1.8;

private:
  PedestrianId id_{0U};
  double x_{0.0};
  double y_{0.0};
  double target_y_{0.0};
  double speed_{1.2};
  PedestrianState state_{PedestrianState::Waiting};
  double wait_timer_{0.0};

  static constexpr double kMaxWaitTime = 3.0;
};

} // namespace core
} // namespace adas

#endif // ADAS_CORE_PEDESTRIAN_HPP
