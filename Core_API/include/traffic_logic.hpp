#ifndef ADAS_CORE_TRAFFIC_LOGIC_HPP
#define ADAS_CORE_TRAFFIC_LOGIC_HPP

#include "ego_vehicle.hpp"
#include "traffic_light.hpp"
#include "traffic_sign.hpp"
#include <set>
#include <vector>
namespace adas
{
namespace core
{

struct StopSignState
{
  bool was_stopped{false};
  double wait_timer{0.0};
  SignId current_sign_id{0U};
  std::set<SignId> processed_signs;
};

class TrafficLogic
{
public:
  static constexpr double kStopSignWaitTime = 0.5;
  static constexpr double kStopLineBuffer = 2.0;
  static constexpr double kDetectionRange = 50.0;
  static constexpr double kMinLightDistance = 20.0;

  /**
   * @brief Check if vehicle should stop for a stop sign
   * @param vehicle Ego vehicle
   * @param signs List of traffic signs
   * @param state Mutable state for stop sign logic
   * @param dt Time step
   * @return true if vehicle should stop
   */
  static bool shouldStopForSign(const EgoVehicle& vehicle,
                                const std::vector<TrafficSign>& signs,
                                StopSignState& state,
                                double dt);

  /**
   * @brief Check if vehicle should stop for a traffic light
   * @param vehicle Ego vehicle
   * @param lights List of traffic lights
   * @return true if vehicle should stop
   */
  static bool shouldStopForLight(const EgoVehicle& vehicle,
                                 const std::vector<TrafficLight>& lights);

  /**
   * @brief Check if a new light position is valid (respecting proximity)
   * @param candidate_x Candidate X position
   * @param existing_lights Existing lights
   * @return true if valid
   */
  static bool isPositionValid(double candidate_x,
                              const std::vector<TrafficLight>& existing_lights);

  /**
   * @brief Compute ACC target speed based on lead vehicle
   * @param ego Ego vehicle (follower)
   * @param lead Lead vehicle (target)
   * @param set_speed Driver set speed
   * @param time_gap Desired time gap in seconds
   * @return Target speed in m/s
   */
  static double computeACCSpeed(const EgoVehicle& ego,
                                const EgoVehicle& lead,
                                double set_speed,
                                double time_gap = 2.0);
};

using LeadVehicle = EgoVehicle;

} // namespace core
} // namespace adas

#endif // ADAS_CORE_TRAFFIC_LOGIC_HPP
