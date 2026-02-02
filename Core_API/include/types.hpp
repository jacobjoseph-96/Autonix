//! @file types.hpp
//! @brief Core type definitions for ADAS simulation
//! @details MISRA C++:2008 and AUTOSAR Adaptive C++ compliant type definitions
//!

#ifndef ADAS_CORE_TYPES_HPP
#define ADAS_CORE_TYPES_HPP

#include <cmath>
#include <cstdint>
#include <optional>

namespace adas
{
namespace core
{

//! @brief Coordinate system representation (x, y, theta)
//! @details Theta is the heading angle in radians, measured counter-clockwise
//! from positive x-axis
//!
struct Coordinate
{
  double x{0.0};     ///< X position in meters
  double y{0.0};     ///< Y position in meters
  double theta{0.0}; ///< Heading angle in radians

  constexpr Coordinate() noexcept = default;

  constexpr Coordinate(double x_pos,
                       double y_pos,
                       double heading = 0.0) noexcept
      : x(x_pos), y(y_pos), theta(heading)
  {
  }

  //! @brief Equality comparison with tolerance
  //!
  [[nodiscard]] constexpr bool equals(const Coordinate& other,
                                      double tolerance = 0.001) const noexcept
  {
    return (std::abs(x - other.x) < tolerance) &&
           (std::abs(y - other.y) < tolerance) &&
           (std::abs(theta - other.theta) < tolerance);
  }
};

/// Alias for position (2D point without heading consideration)
using Position = Coordinate;

//! @brief Traffic sign type enumeration
//! @details AUTOSAR compliant enum class with explicit underlying type
//!
enum class TrafficSignType : std::uint8_t
{
  SpeedLimit = 0U,
  Stop = 1U,
  Yield = 2U
};

//! @brief Traffic light state enumeration
//! @details AUTOSAR compliant enum class with explicit underlying type
//!
enum class TrafficLightState : std::uint8_t
{
  Red = 0U,
  Yellow = 1U,
  Green = 2U
};

//! @brief Turn signal state enumeration
//! @details AUTOSAR compliant enum class with explicit underlying type
//!
enum class TurnSignalState : std::uint8_t
{
  Off = 0U,
  Left = 1U,
  Right = 2U,
  Hazard = 3U
};

//! @brief Lane change direction enumeration
//! @details AUTOSAR compliant enum class with explicit underlying type
//!
enum class LaneChangeDirection : std::uint8_t
{
  None = 0U,
  Left = 1U,
  Right = 2U
};

//! @brief Result of gap analysis for lane change
//!
struct GapAnalysisResult
{
  bool is_safe{false};           ///< Whether lane change is safe
  double front_gap{0.0};         ///< Distance to vehicle ahead in target lane
  double rear_gap{0.0};          ///< Distance to vehicle behind in target lane
  double min_safe_gap{10.0};     ///< Minimum required gap
  bool target_lane_exists{true}; ///< Whether target lane exists

  constexpr GapAnalysisResult() noexcept = default;

  constexpr GapAnalysisResult(bool safe,
                              double front,
                              double rear,
                              double min_gap,
                              bool exists) noexcept
      : is_safe(safe), front_gap(front), rear_gap(rear), min_safe_gap(min_gap),
        target_lane_exists(exists)
  {
  }

  //! @brief Check if gap analysis result indicates safe lane change
  //! @return true if lane change is safe
  //!
  [[nodiscard]] constexpr bool isSafe() const noexcept
  {
    return is_safe && target_lane_exists;
  }
};

//! @brief Traffic light identifier type
//!
using TrafficLightId = std::uint32_t;

//! @brief Lane identifier type
//! @details Using MISRA-compliant fixed-width integer type
//!
using LaneId = std::uint32_t;

//! @brief Road segment identifier type
//!
using SegmentId = std::uint32_t;

//! @brief Sign identifier type
//!
using SignId = std::uint32_t;

//! @brief Constants for perception calculations
//!
namespace constants
{
/// Field of View angle in radians (120 degrees)
constexpr double kFieldOfViewRadians = (120.0 * M_PI) / 180.0;

/// Half FoV for symmetric cone calculation
constexpr double kHalfFovRadians = kFieldOfViewRadians / 2.0;

/// Maximum perception range in meters
constexpr double kMaxPerceptionRange = 100.0;

/// Tolerance for floating point comparisons
constexpr double kFloatTolerance = 0.001;
} // namespace constants

//! @brief Convert traffic sign type to string representation
//! @param type The traffic sign type
//! @return String representation of the sign type
//!
[[nodiscard]] inline const char *
trafficSignTypeToString(TrafficSignType type) noexcept
{
  switch (type)
  {
  case TrafficSignType::SpeedLimit:
    return "SpeedLimit";
  case TrafficSignType::Stop:
    return "Stop";
  case TrafficSignType::Yield:
    return "Yield";
  default:
    return "Unknown";
  }
}

} // namespace core
} // namespace adas

#endif // ADAS_CORE_TYPES_HPP
