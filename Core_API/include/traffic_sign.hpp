//! @file traffic_sign.hpp
//! @brief Traffic sign definition and factory
//!

#ifndef ADAS_CORE_TRAFFIC_SIGN_HPP
#define ADAS_CORE_TRAFFIC_SIGN_HPP

#include "types.hpp"
#include <optional>
#include <string>

namespace adas
{
namespace core
{

//! @brief Traffic sign entity
//! @details Immutable representation of a traffic sign in the world
//!
class TrafficSign
{
public:
  //! @brief Factory function for safe construction
  //! @param id Sign identifier
  //! @param type Sign type
  //! @param position Sign position
  //! @param lane_id Associated lane ID
  //! @param value Optional value (e.g., speed limit value)
  //! @return Constructed traffic sign
  //!
  [[nodiscard]] static TrafficSign
  create(SignId id,
         TrafficSignType type,
         const Position& position,
         LaneId lane_id,
         std::optional<std::uint32_t> value = std::nullopt) noexcept;

  // Default constructor for container compatibility
  TrafficSign() noexcept = default;

  //! @brief Get sign ID
  //!
  [[nodiscard]] constexpr SignId getId() const noexcept
  {
    return id_;
  }

  //! @brief Get sign type
  //!
  [[nodiscard]] constexpr TrafficSignType getType() const noexcept
  {
    return type_;
  }

  //! @brief Get sign position
  //!
  [[nodiscard]] constexpr const Position& getPosition() const noexcept
  {
    return position_;
  }

  //! @brief Get associated lane ID
  //!
  [[nodiscard]] constexpr LaneId getLaneId() const noexcept
  {
    return lane_id_;
  }

  //! @brief Get optional value (e.g., speed limit)
  //!
  [[nodiscard]] constexpr std::optional<std::uint32_t> getValue() const noexcept
  {
    return value_;
  }

  //! @brief Get string representation of sign type
  //!
  [[nodiscard]] const char *getTypeString() const noexcept;

  //! @brief Check if sign is valid (has been properly constructed)
  //!
  [[nodiscard]] constexpr bool isValid() const noexcept
  {
    return id_ != 0U;
  }

private:
  TrafficSign(SignId id,
              TrafficSignType type,
              const Position& position,
              LaneId lane_id,
              std::optional<std::uint32_t> value) noexcept;

  SignId id_{0U};
  TrafficSignType type_{TrafficSignType::Stop};
  Position position_{};
  LaneId lane_id_{0U};
  std::optional<std::uint32_t> value_{std::nullopt};
};

//! @brief Parse traffic sign type from string
//! @param type_str String representation (case insensitive)
//! @return Optional sign type if valid
//!
[[nodiscard]] std::optional<TrafficSignType>
parseTrafficSignType(const std::string& type_str) noexcept;

} // namespace core
} // namespace adas

#endif // ADAS_CORE_TRAFFIC_SIGN_HPP
