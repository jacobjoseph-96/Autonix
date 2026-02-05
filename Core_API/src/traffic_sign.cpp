/**
 * @file traffic_sign.cpp
 * @brief Traffic sign implementation
 */

#include "traffic_sign.hpp"
#include <algorithm>
#include <cctype>

namespace adas
{
namespace core
{

TrafficSign::TrafficSign(SignId id,
                         TrafficSignType type,
                         const Position& position,
                         LaneId lane_id,
                         std::optional<std::uint32_t> value) noexcept
    : id_(id), type_(type), position_(position), lane_id_(lane_id),
      value_(value)
{
}

TrafficSign TrafficSign::create(SignId id,
                                TrafficSignType type,
                                const Position& position,
                                LaneId lane_id,
                                std::optional<std::uint32_t> value) noexcept
{
  return TrafficSign(id, type, position, lane_id, value);
}

const char *TrafficSign::getTypeString() const noexcept
{
  return trafficSignTypeToString(type_);
}

std::optional<TrafficSignType>
parseTrafficSignType(const std::string& type_str) noexcept
{
  // Convert to lowercase for comparison
  std::string lower_str = type_str;
  std::transform(lower_str.begin(),
                 lower_str.end(),
                 lower_str.begin(),
                 [](unsigned char c)
                 { return static_cast<char>(std::tolower(c)); });

  if (lower_str == "speedlimit" || lower_str == "speed_limit")
  {
    return TrafficSignType::SpeedLimit;
  }
  if (lower_str == "stop")
  {
    return TrafficSignType::Stop;
  }
  if (lower_str == "yield")
  {
    return TrafficSignType::Yield;
  }

  return std::nullopt;
}

} // namespace core
} // namespace adas
