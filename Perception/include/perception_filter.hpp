//! @file perception_filter.hpp
//! @brief Traffic sign perception filtering with FoV and lane relevance
//!

#ifndef ADAS_PERCEPTION_PERCEPTION_FILTER_HPP
#define ADAS_PERCEPTION_PERCEPTION_FILTER_HPP

#include "ego_vehicle.hpp"
#include "road_segment.hpp"
#include "safe_distance_calculator.hpp"
#include "traffic_light.hpp"
#include "traffic_sign.hpp"
#include "types.hpp"
#include <cmath>
#include <vector>

namespace adas
{
namespace perception
{

//! @brief Detection result with distance and relevance
//!
struct DetectionResult
{
  core::TrafficSign sign;
  double distance{0.0};
  bool is_relevant{false};
  bool in_fov{false};

  DetectionResult() noexcept = default;

  DetectionResult(const core::TrafficSign& s,
                  double d,
                  bool rel,
                  bool fov) noexcept
      : sign(s), distance(d), is_relevant(rel), in_fov(fov)
  {
  }

  //! @brief Get speed limit value if this is a speed limit sign
  //! @return Speed limit in km/h if applicable, std::nullopt otherwise
  //!
  [[nodiscard]] std::optional<std::uint32_t> getSpeedLimit() const noexcept
  {
    if (sign.getType() == core::TrafficSignType::SpeedLimit)
    {
      return sign.getValue();
    }
    return std::nullopt;
  }
};

//! @brief Traffic light detection result with distance and relevance
//!
struct TrafficLightDetectionResult
{
  core::TrafficLightId id{0U};
  core::TrafficLightState state{core::TrafficLightState::Red};
  double distance{0.0};
  bool is_relevant{false};
  bool in_fov{false};

  TrafficLightDetectionResult() noexcept = default;

  TrafficLightDetectionResult(core::TrafficLightId light_id,
                              core::TrafficLightState light_state,
                              double d,
                              bool rel,
                              bool fov) noexcept
      : id(light_id), state(light_state), distance(d), is_relevant(rel),
        in_fov(fov)
  {
  }
};

//! @brief Perception filter for traffic sign detection
//! @details Filters signs based on 120° FoV cone and lane relevance
//!
class PerceptionFilter
{
public:
  //! @brief Construct perception filter
  //! @param fov_radians Field of view angle in radians (default 120°)
  //! @param max_range Maximum perception range in meters
  //!
  explicit PerceptionFilter(
      double fov_radians = core::constants::kFieldOfViewRadians,
      double max_range = core::constants::kMaxPerceptionRange) noexcept;

  //! @brief Filter signs based on ego vehicle state
  //! @param ego Ego vehicle state
  //! @param signs All traffic signs in the world
  //! @param road Road segment for lane checking
  //! @return Vector of detection results for signs in FoV
  //!
  [[nodiscard]] std::vector<DetectionResult>
  filter(const core::EgoVehicle& ego,
         const std::vector<core::TrafficSign>& signs,
         const core::RoadSegment& road) const;

  //! @brief Check if a sign is within the field of view
  //! @param ego Ego vehicle position and heading
  //! @param sign_position Sign position
  //! @return true if sign is within FoV cone
  //!
  [[nodiscard]] bool
  isInFieldOfView(const core::EgoVehicle& ego,
                  const core::Position& sign_position) const noexcept;

  //! @brief Check if a sign is relevant based on lane
  //! @param ego_lane_id Ego vehicle's current lane
  //! @param sign_lane_id Sign's associated lane
  //! @param road Road segment for adjacency check
  //! @return true if sign is in current or adjacent lane
  //!
  [[nodiscard]] bool
  isLaneRelevant(core::LaneId ego_lane_id,
                 core::LaneId sign_lane_id,
                 const core::RoadSegment& road) const noexcept;

  //! @brief Get field of view angle
  //!
  [[nodiscard]] constexpr double getFovRadians() const noexcept
  {
    return fov_radians_;
  }

  //! @brief Get maximum perception range
  //!
  [[nodiscard]] constexpr double getMaxRange() const noexcept
  {
    return max_range_;
  }

private:
  double fov_radians_;
  double max_range_;
  double half_fov_;

  //! @brief Calculate angle from ego to target
  //! @return Angle in radians [-π, π]
  //!
  [[nodiscard]] static double
  calculateBearing(const core::Coordinate& from,
                   const core::Coordinate& to) noexcept;

  //! @brief Normalize angle to [-π, π]
  //!
  [[nodiscard]] static double normalizeAngle(double angle) noexcept;
};

} // namespace perception
} // namespace adas

#endif // ADAS_PERCEPTION_PERCEPTION_FILTER_HPP
