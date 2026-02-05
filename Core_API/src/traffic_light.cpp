/**
 * @file traffic_light.cpp
 * @brief Traffic light implementation
 */

#include "traffic_light.hpp"

namespace adas
{
namespace core
{

TrafficLight::TrafficLight(TrafficLightId id,
                           const Position& position,
                           LaneId lane_id,
                           TrafficLightState initial_state,
                           double cycle_duration) noexcept
    : id_(id), position_(position), lane_id_(lane_id), state_(initial_state),
      cycle_duration_(cycle_duration), state_timer_(0.0)
{
  // Calculate state duration based on current state
  switch (state_)
  {
  case TrafficLightState::Red:
    state_duration_ = cycle_duration_ * kRedFraction;
    break;
  case TrafficLightState::Yellow:
    state_duration_ = cycle_duration_ * kYellowFraction;
    break;
  case TrafficLightState::Green:
    state_duration_ = cycle_duration_ * kGreenFraction;
    break;
  }
}

TrafficLight TrafficLight::create(TrafficLightId id,
                                  const Position& position,
                                  LaneId lane_id,
                                  TrafficLightState initial_state,
                                  double cycle_duration) noexcept
{
  return TrafficLight(id, position, lane_id, initial_state, cycle_duration);
}

void TrafficLight::update(double delta_time) noexcept
{
  state_timer_ += delta_time;

  while (state_timer_ >= state_duration_)
  {
    state_timer_ -= state_duration_;
    advanceState();
  }
}

void TrafficLight::advanceState() noexcept
{
  switch (state_)
  {
  case TrafficLightState::Red:
    state_ = TrafficLightState::Green;
    state_duration_ = cycle_duration_ * kGreenFraction;
    break;
  case TrafficLightState::Green:
    state_ = TrafficLightState::Yellow;
    state_duration_ = cycle_duration_ * kYellowFraction;
    break;
  case TrafficLightState::Yellow:
    state_ = TrafficLightState::Red;
    state_duration_ = cycle_duration_ * kRedFraction;
    break;
  }
}

} // namespace core
} // namespace adas
