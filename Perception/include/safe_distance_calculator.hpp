//! @file safe_distance_calculator.hpp
//! @brief No-throw guaranteed distance calculator
//!

#ifndef ADAS_PERCEPTION_SAFE_DISTANCE_CALCULATOR_HPP
#define ADAS_PERCEPTION_SAFE_DISTANCE_CALCULATOR_HPP

#include "types.hpp"
#include <cmath>
#include <optional>

namespace adas
{
namespace perception
{

//! @brief Safe distance calculator with noexcept guarantee
//! @details Implements Euclidean distance calculation with no-throw guarantees
//! as required by AUTOSAR safety standards
//!
class SafeDistanceCalculator
{
public:
  SafeDistanceCalculator() noexcept = default;

  //! @brief Calculate Euclidean distance between two coordinates
  //! @param p1 First coordinate
  //! @param p2 Second coordinate
  //! @return Distance in meters (always non-negative)
  //! @note d = sqrt((x2-x1)^2 + (y2-y1)^2)
  //!
  [[nodiscard]] static constexpr double
  calculate(const core::Coordinate& p1, const core::Coordinate& p2) noexcept
  {
    const double dx = p2.x - p1.x;
    const double dy = p2.y - p1.y;
    return std::sqrt((dx * dx) + (dy * dy));
  }

  //! @brief Calculate distance between two points (x, y coordinates only)
  //! @param x1 First point X
  //! @param y1 First point Y
  //! @param x2 Second point X
  //! @param y2 Second point Y
  //! @return Distance in meters
  //!
  [[nodiscard]] static constexpr double
  calculate(double x1, double y1, double x2, double y2) noexcept
  {
    const double dx = x2 - x1;
    const double dy = y2 - y1;
    return std::sqrt((dx * dx) + (dy * dy));
  }

  //! @brief Calculate squared distance (avoids sqrt for comparisons)
  //! @param p1 First coordinate
  //! @param p2 Second coordinate
  //! @return Squared distance
  //!
  [[nodiscard]] static constexpr double
  calculateSquared(const core::Coordinate& p1,
                   const core::Coordinate& p2) noexcept
  {
    const double dx = p2.x - p1.x;
    const double dy = p2.y - p1.y;
    return (dx * dx) + (dy * dy);
  }

  //! @brief Check if distance is within threshold
  //! @param p1 First coordinate
  //! @param p2 Second coordinate
  //! @param threshold Maximum allowed distance
  //! @return true if distance <= threshold
  //!
  [[nodiscard]] static constexpr bool
  isWithinDistance(const core::Coordinate& p1,
                   const core::Coordinate& p2,
                   double threshold) noexcept
  {
    // Use squared comparison to avoid sqrt
    const double threshold_squared = threshold * threshold;
    return calculateSquared(p1, p2) <= threshold_squared;
  }

  //! @brief Safe distance calculation with validation
  //! @param p1 First coordinate
  //! @param p2 Second coordinate
  //! @return Optional distance (nullopt if coordinates contain NaN/Inf)
  //!
  [[nodiscard]] static std::optional<double>
  calculateSafe(const core::Coordinate& p1, const core::Coordinate& p2) noexcept
  {
    // Validate inputs
    if (!std::isfinite(p1.x) || !std::isfinite(p1.y) || !std::isfinite(p2.x) ||
        !std::isfinite(p2.y))
    {
      return std::nullopt;
    }
    return calculate(p1, p2);
  }

  //! @brief Compare two distances within tolerance
  //! @param actual Calculated distance
  //! @param expected Expected distance
  //! @param tolerance Comparison tolerance (default 0.001)
  //! @return true if |actual - expected| < tolerance
  //!
  [[nodiscard]] static constexpr bool compareWithTolerance(
      double actual,
      double expected,
      double tolerance = core::constants::kFloatTolerance) noexcept
  {
    return std::abs(actual - expected) < tolerance;
  }
};

} // namespace perception
} // namespace adas

#endif // ADAS_PERCEPTION_SAFE_DISTANCE_CALCULATOR_HPP
