/**
 * @file safe_distance_calculator.cpp
 * @brief Safe distance calculator implementation (mostly header-only, for
 * linking)
 */

#include "safe_distance_calculator.hpp"

// Implementation is header-only due to constexpr/noexcept requirements
// This file exists for proper library linking

namespace adas
{
namespace perception
{

// Explicit template instantiation not needed for this class
// All methods are constexpr and defined in header

} // namespace perception
} // namespace adas
