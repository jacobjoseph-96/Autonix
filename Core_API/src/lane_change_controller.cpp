/**
 * @file lane_change_controller.cpp
 * @brief Lane change controller implementation
 */

#include "lane_change_controller.hpp"

namespace adas
{
namespace core
{

bool LaneChangeController::requestLaneChange(
    EgoVehicle& vehicle,
    LaneChangeDirection direction,
    const RoadSegment& road,
    const GapAnalysisResult& gap_result) noexcept
{
  // Cannot start if already active
  if (state_ != LaneChangeState::Idle)
  {
    return false;
  }

  // Cannot change lanes if moving too slowly
  if (vehicle.getSpeed() < kMinSpeedForLaneChange)
  {
    return false;
  }

  // Check if gap is safe
  if (!gap_result.isSafe())
  {
    return false;
  }

  // Calculate target lane
  const LaneId current_lane = vehicle.getLaneId();
  target_lane_id_ = calculateTargetLane(current_lane, direction);

  // Verify target lane exists
  const auto target_boundary = road.getLaneBoundary(target_lane_id_);
  if (!target_boundary.has_value())
  {
    return false;
  }

  // Store lane change parameters
  direction_ = direction;
  start_y_ = vehicle.getY();
  target_y_ = target_boundary->centerY();
  timer_ = 0.0;

  // Start signaling
  state_ = LaneChangeState::Signaling;
  vehicle.setTurnSignal(direction == LaneChangeDirection::Left
                            ? TurnSignalState::Left
                            : TurnSignalState::Right);

  return true;
}

void LaneChangeController::update(EgoVehicle& vehicle,
                                  const RoadSegment& /*road*/,
                                  double delta_time) noexcept
{
  switch (state_)
  {
  case LaneChangeState::Idle:
    // Nothing to do
    break;

  case LaneChangeState::Signaling:
    timer_ += delta_time;
    if (timer_ >= kSignalLeadTime)
    {
      // Transition to executing
      state_ = LaneChangeState::Executing;
      timer_ = 0.0;
      vehicle.startLaneChange(direction_, target_lane_id_, start_y_, target_y_);
    }
    break;

  case LaneChangeState::Executing:
  {
    // Update the vehicle's lane change progress
    const bool completed =
        vehicle.updateLaneChange(delta_time, kLaneChangeDuration);
    if (completed)
    {
      state_ = LaneChangeState::Completing;
    }
  }
  break;

  case LaneChangeState::Completing:
    // Finalize lane change
    vehicle.completeLaneChange();
    state_ = LaneChangeState::Idle;
    direction_ = LaneChangeDirection::None;
    target_lane_id_ = 0U;
    break;
  }
}

void LaneChangeController::cancel(EgoVehicle& vehicle) noexcept
{
  if (state_ != LaneChangeState::Idle)
  {
    vehicle.cancelLaneChange();
    state_ = LaneChangeState::Idle;
    direction_ = LaneChangeDirection::None;
    target_lane_id_ = 0U;
    timer_ = 0.0;
  }
}

LaneId LaneChangeController::calculateTargetLane(
    LaneId current_lane, LaneChangeDirection direction) noexcept
{
  if (direction == LaneChangeDirection::Left)
  {
    // Left = higher lane ID (in this road model)
    return current_lane + 1U;
  }
  else if (direction == LaneChangeDirection::Right)
  {
    // Right = lower lane ID
    return (current_lane > 1U) ? current_lane - 1U : 1U;
  }
  return current_lane;
}

} // namespace core
} // namespace adas
