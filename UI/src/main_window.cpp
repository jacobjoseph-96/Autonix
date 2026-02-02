/**
 * @file main_window.cpp
 * @brief Main window implementation
 */

#include "main_window.hpp"
#include <QApplication>
#include <QMenuBar>
#include <QPushButton>
#include <QStatusBar>
#include <QToolBar>
#include <algorithm>
#include <chrono>

namespace adas
{
namespace ui
{

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), perspective_dock_(std::make_unique<QDockWidget>(
                               "Perspective View", this)),
      overlay_dock_(std::make_unique<QDockWidget>("Detection Overlay", this)),
      logs_dock_(std::make_unique<QDockWidget>("Diagnostic Logs", this)),
      perspective_view_(std::make_unique<PerspectiveView>()),
      detection_overlay_(std::make_unique<DetectionOverlay>()),
      diagnostic_logs_(std::make_unique<DiagnosticLogs>()),
      simulation_timer_(std::make_unique<QTimer>(this)),
      ego_vehicle_(core::Coordinate{0.0, 7.5, 0.0}, 1U),
      road_segment_(1U, -10.0, 150.0), perception_filter_(),
      random_engine_(static_cast<std::uint32_t>(
          std::chrono::steady_clock::now().time_since_epoch().count()))
{
  setupUi();
  setupToolBar();
  setupDockWidgets();
  setupSimulation();
  initializeWorld();
}

void MainWindow::setupUi()
{
  setWindowTitle("ADAS Simulation - Qt6");
  resize(1400, 900);

  setStyleSheet("QMainWindow {"
                "  background-color: #0a0a14;"
                "}"
                "QDockWidget {"
                "  color: #ffffff;"
                "  font-weight: bold;"
                "  titlebar-close-icon: none;"
                "}"
                "QDockWidget::title {"
                "  background-color: #16213e;"
                "  padding: 8px;"
                "  text-align: center;"
                "}");

  // Menu bar
  auto *file_menu = menuBar()->addMenu("&File");
  file_menu->addAction("&Reset", this, &MainWindow::resetSimulation);
  file_menu->addSeparator();
  file_menu->addAction("E&xit", qApp, &QApplication::quit);

  auto *sim_menu = menuBar()->addMenu("&Simulation");
  sim_menu->addAction("&Start", this, &MainWindow::startSimulation);
  sim_menu->addAction("Sto&p", this, &MainWindow::stopSimulation);

  // Status bar
  statusBar()->showMessage("Ready");
  statusBar()->setStyleSheet("color: #aaa;");
}

void MainWindow::setupToolBar()
{
  tool_bar_ = std::make_unique<QToolBar>("Simulation Controls", this);
  tool_bar_->setMovable(false);
  tool_bar_->setStyleSheet("QToolBar {"
                           "  background-color: #16213e;"
                           "  border: none;"
                           "  spacing: 10px;"
                           "  padding: 5px;"
                           "}"
                           "QPushButton {"
                           "  background-color: #1a1a2e;"
                           "  color: white;"
                           "  border: 1px solid #4a4a6a;"
                           "  border-radius: 6px;"
                           "  padding: 8px 16px;"
                           "  font-weight: bold;"
                           "  min-width: 80px;"
                           "}"
                           "QPushButton:hover {"
                           "  background-color: #2a2a4e;"
                           "  border-color: #66a5f5;"
                           "}"
                           "QPushButton:pressed {"
                           "  background-color: #0f0f1e;"
                           "}"
                           "QPushButton#restartBtn {"
                           "  background-color: #1e3a5f;"
                           "  border-color: #42a5f5;"
                           "}"
                           "QPushButton#restartBtn:hover {"
                           "  background-color: #2a4a7f;"
                           "}");

  auto *start_btn = new QPushButton("▶ Start");
  auto *stop_btn = new QPushButton("⏹ Stop");
  auto *restart_btn = new QPushButton("🔄 Restart");
  restart_btn->setObjectName("restartBtn");

  connect(start_btn, &QPushButton::clicked, this, &MainWindow::startSimulation);
  connect(stop_btn, &QPushButton::clicked, this, &MainWindow::stopSimulation);
  connect(restart_btn,
          &QPushButton::clicked,
          this,
          [this]()
          {
            resetSimulation();
            startSimulation();
          });

  tool_bar_->addWidget(start_btn);
  tool_bar_->addWidget(stop_btn);
  tool_bar_->addWidget(restart_btn);

  // NPC toggle checkbox
  tool_bar_->addSeparator();
  npc_toggle_ = new QCheckBox("Enable NPCs");
  npc_toggle_->setStyleSheet("QCheckBox {"
                             "  color: white;"
                             "  font-weight: bold;"
                             "  spacing: 8px;"
                             "}"
                             "QCheckBox::indicator {"
                             "  width: 18px;"
                             "  height: 18px;"
                             "}");
  connect(
      npc_toggle_, &QCheckBox::stateChanged, this, &MainWindow::onNpcToggle);
  tool_bar_->addWidget(npc_toggle_);

  // Pedestrian toggle checkbox
  pedestrians_toggle_ = new QCheckBox("Enable Pedestrians");
  pedestrians_toggle_->setStyleSheet("QCheckBox {"
                                     "  color: white;"
                                     "  font-weight: bold;"
                                     "  spacing: 8px;"
                                     "}"
                                     "QCheckBox::indicator {"
                                     "  width: 18px;"
                                     "  height: 18px;"
                                     "}");
  connect(pedestrians_toggle_,
          &QCheckBox::stateChanged,
          this,
          &MainWindow::onPedestriansToggle);
  tool_bar_->addWidget(pedestrians_toggle_);

  addToolBar(Qt::TopToolBarArea, tool_bar_.get());
}

void MainWindow::setupDockWidgets()
{
  // Configure dock widgets
  perspective_dock_->setWidget(perspective_view_.get());
  perspective_dock_->setFeatures(QDockWidget::DockWidgetMovable |
                                 QDockWidget::DockWidgetFloatable);

  overlay_dock_->setWidget(detection_overlay_.get());
  overlay_dock_->setFeatures(QDockWidget::DockWidgetMovable |
                             QDockWidget::DockWidgetFloatable);
  overlay_dock_->setMinimumWidth(280);
  overlay_dock_->setMaximumWidth(350);

  logs_dock_->setWidget(diagnostic_logs_.get());
  logs_dock_->setFeatures(QDockWidget::DockWidgetMovable |
                          QDockWidget::DockWidgetFloatable);
  logs_dock_->setMinimumHeight(150);
  logs_dock_->setMaximumHeight(250);

  // Add docks to main window
  setCentralWidget(nullptr);
  addDockWidget(Qt::LeftDockWidgetArea, perspective_dock_.get());
  addDockWidget(Qt::RightDockWidgetArea, overlay_dock_.get());
  addDockWidget(Qt::BottomDockWidgetArea, logs_dock_.get());

  // Connect signals
  connect(detection_overlay_.get(),
          &DetectionOverlay::relevantSignDetected,
          this,
          [this](const QString& type, double distance)
          {
            diagnostic_logs_->logInfo(
                QString("Relevant sign detected: %1 at %2m")
                    .arg(type)
                    .arg(distance, 0, 'f', 1));
          });
}

void MainWindow::setupSimulation()
{
  connect(simulation_timer_.get(),
          &QTimer::timeout,
          this,
          &MainWindow::onSimulationTick);

  simulation_timer_->setInterval(static_cast<int>(1000.0 / kFrameRate));
}

void MainWindow::initializeWorld()
{
  // Create road lanes
  road_segment_.addLane(1U, core::LaneBoundary{10.0, 5.0}); // Lane 1: y = 5-10
  road_segment_.addLane(2U, core::LaneBoundary{5.0, 0.0});  // Lane 2: y = 0-5
  road_segment_.addLane(3U, core::LaneBoundary{0.0, -5.0}); // Lane 3: y = -5-0

  // Generate traffic elements
  // Generate traffic elements (lights first so signs can check proximity)
  generateTrafficLights();
  generateTrafficSigns();

  // Update views
  perspective_view_->updateRoad(road_segment_);
  perspective_view_->updateSigns(traffic_signs_);
  perspective_view_->updateTrafficLights(traffic_lights_,
                                         ego_vehicle_.getLaneId());
  perspective_view_->updateEgoVehicle(ego_vehicle_);
  perspective_view_->updateFovVisualization(ego_vehicle_.getPosition(),
                                            ego_vehicle_.getHeading(),
                                            perception_filter_.getFovRadians(),
                                            perception_filter_.getMaxRange());

  diagnostic_logs_->logInfo("World initialized with 3 lanes");
  diagnostic_logs_->logInfo(
      QString("Generated %1 traffic signs").arg(traffic_signs_.size()));
  diagnostic_logs_->logInfo(
      QString("Generated %1 traffic lights").arg(traffic_lights_.size()));
}

void MainWindow::generateTrafficSigns()
{
  traffic_signs_.clear();

  // Use seeded random distribution (MISRA compliant)
  std::uniform_real_distribution<double> x_dist(10.0, 120.0);
  std::uniform_real_distribution<double> y_dist(-4.0, 9.0);
  std::uniform_int_distribution<int> type_dist(0, 2);
  std::uniform_int_distribution<std::uint32_t> lane_dist(1U, 3U);

  constexpr std::size_t kSignCount = 8U;
  constexpr double kMinDistanceToLight =
      20.0; // Minimum meters from any traffic light

  for (std::uint32_t i = 1U; i <= kSignCount; ++i)
  {
    // Try to find a valid position up to 10 times
    for (int attempt = 0; attempt < 10; ++attempt)
    {
      const auto type =
          static_cast<core::TrafficSignType>(type_dist(random_engine_));
      const double x = x_dist(random_engine_);
      const double y = y_dist(random_engine_);
      const core::LaneId lane_id = lane_dist(random_engine_);

      // Check distance to all traffic lights
      bool too_close = false;
      for (const auto& light : traffic_lights_)
      {
        if (std::abs(light.getX() - x) < kMinDistanceToLight)
        {
          too_close = true;
          break;
        }
      }

      if (!too_close)
      {
        traffic_signs_.push_back(
            core::TrafficSign::create(i, type, core::Position{x, y}, lane_id));
        break; // Successfully added sign
      }
    }
  }

  // Add a specific stop sign ahead for demonstration
  traffic_signs_.push_back(core::TrafficSign::create(
      100U, core::TrafficSignType::Stop, core::Position{50.0, 7.5}, 1U));
}

void MainWindow::startSimulation()
{
  if (!is_running_)
  {
    is_running_ = true;
    simulation_timer_->start();
    diagnostic_logs_->logInfo("Simulation started");
    statusBar()->showMessage("Simulation running...");
  }
}

void MainWindow::stopSimulation()
{
  if (is_running_)
  {
    is_running_ = false;
    simulation_timer_->stop();
    diagnostic_logs_->logInfo("Simulation stopped");
    statusBar()->showMessage("Simulation paused");
  }
}

void MainWindow::resetSimulation()
{
  stopSimulation();

  ego_vehicle_ = core::EgoVehicle(core::Coordinate{0.0, 7.5, 0.0}, 1U);
  simulation_time_ = 0.0;

  // Re-seed random engine
  random_engine_.seed(static_cast<std::uint32_t>(
      std::chrono::steady_clock::now().time_since_epoch().count()));

  generateTrafficLights();
  generateTrafficSigns();

  perspective_view_->updateSigns(traffic_signs_);
  perspective_view_->updateTrafficLights(traffic_lights_,
                                         ego_vehicle_.getLaneId());
  perspective_view_->updateEgoVehicle(ego_vehicle_);
  detection_overlay_->clear();
  stop_sign_state_ = core::StopSignState{};

  // Reset NPCs if enabled
  if (npcs_enabled_)
  {
    spawnNPCs();
  }

  // Reset pedestrians if enabled
  if (pedestrians_enabled_)
  {
    spawnPedestrians();
  }

  diagnostic_logs_->logInfo("Simulation reset");
  statusBar()->showMessage("Ready");
}

void MainWindow::onSimulationTick()
{
  simulation_time_ += kDeltaTime;

  // Update traffic lights
  updateTrafficLights();

  // Check if vehicle should stop for red light or stop sign
  const bool should_stop_light = shouldStopForLight();
  const bool should_stop_sign = shouldStopForSign();
  bool should_stop = should_stop_light || should_stop_sign;

  if (should_stop_light && !was_stopped_for_light_)
  {
    // Just started stopping for light
    diagnostic_logs_->logWarning("Vehicle stopping for RED/YELLOW light");
    was_stopped_for_light_ = true;
  }
  else if (!should_stop_light && was_stopped_for_light_)
  {
    // Just started moving again from light
    diagnostic_logs_->logInfo("Light turned GREEN - Vehicle resuming");
    was_stopped_for_light_ = false;
  }

  // Check for NPC vehicles ahead in the same lane
  double target_speed = 5.0;                     // Default 5 m/s = 18 km/h
  constexpr double kSafeFollowingDistance = 8.0; // meters
  constexpr double kMinFollowingDistance =
      3.0; // Stop completely at this distance

  if (npcs_enabled_)
  {
    for (const auto& npc : npc_vehicles_)
    {
      // Check if NPC is in the same lane
      if (npc.getLaneId() == ego_vehicle_.getLaneId())
      {
        const double distance = npc.getX() - ego_vehicle_.getX();

        // Only consider NPCs ahead of us
        if (distance > 0.0 && distance < kSafeFollowingDistance)
        {
          if (distance < kMinFollowingDistance)
          {
            // Too close - stop completely
            should_stop = true;
            diagnostic_logs_->logWarning(
                QString("Stopping behind NPC - distance: %1m")
                    .arg(distance, 0, 'f', 1));
          }
          else
          {
            // Slow down proportionally to distance
            const double speed_factor =
                (distance - kMinFollowingDistance) /
                (kSafeFollowingDistance - kMinFollowingDistance);
            target_speed =
                std::min(target_speed, npc.getSpeed() * speed_factor);
          }
        }
      }
    }
  }

  // Emergency braking for pedestrians
  if (shouldEmergencyBrake())
  {
    should_stop = true;
    diagnostic_logs_->logWarning("EMERGENCY BRAKE - Pedestrian detected!");
  }

  if (should_stop)
  {
    ego_vehicle_.setSpeed(0.0);
  }
  else
  {
    ego_vehicle_.setSpeed(target_speed);
  }

  ego_vehicle_.update(kDeltaTime);

  // Update lane change
  updateLaneChange();

  // Update NPC vehicles
  updateNPCs();

  // Update pedestrians
  updatePedestrians();

  // Update perception
  updatePerception();
  updateTrafficLightDetections();

  // Update visualizations
  perspective_view_->updateEgoVehicle(ego_vehicle_);
  perspective_view_->updateTrafficLights(traffic_lights_,
                                         ego_vehicle_.getLaneId());
  perspective_view_->updateFovVisualization(ego_vehicle_.getPosition(),
                                            ego_vehicle_.getHeading(),
                                            perception_filter_.getFovRadians(),
                                            perception_filter_.getMaxRange());

  // Check for end of road
  if (ego_vehicle_.getX() > road_segment_.getEndX())
  {
    diagnostic_logs_->logInfo("End of road segment reached");
    resetSimulation();
  }
}

void MainWindow::updatePerception()
{
  const auto detections =
      perception_filter_.filter(ego_vehicle_, traffic_signs_, road_segment_);

  // Collect detected sign IDs
  std::vector<core::SignId> detected_ids;
  detected_ids.reserve(detections.size());
  for (const auto& det : detections)
  {
    detected_ids.push_back(det.sign.getId());
  }

  // Update UI
  perspective_view_->updateSigns(traffic_signs_, detected_ids);
  detection_overlay_->updateDetections(detections);

  // Check for safety violations (approaching stop sign too fast)
  for (const auto& det : detections)
  {
    if (det.is_relevant && det.sign.getType() == core::TrafficSignType::Stop &&
        det.distance < 10.0 && ego_vehicle_.getSpeed() > 2.0)
    {
      diagnostic_logs_->logSafetyViolation(
          QString("Approaching STOP sign at %1m with speed %2 m/s")
              .arg(det.distance, 0, 'f', 1)
              .arg(ego_vehicle_.getSpeed(), 0, 'f', 1));
    }
  }
}

void MainWindow::generateTrafficLights()
{
  traffic_lights_.clear();

  // Random distributions for traffic light generation
  std::uniform_real_distribution<double> pos_dist(25.0, 140.0);
  std::uniform_real_distribution<double> cycle_dist(5.0, 12.0);
  std::uniform_int_distribution<int> state_dist(0, 2);

  constexpr std::size_t kNumRelevantLights = 5U;
  constexpr std::size_t kNumIrrelevantLights = 3U;

  core::TrafficLightId light_id = 1U;

  // Generate relevant lights (in lane 1 - ego vehicle's lane)
  for (std::size_t i = 0; i < kNumRelevantLights; ++i)
  {
    for (int attempt = 0; attempt < 20; ++attempt)
    {
      const double x_pos = pos_dist(random_engine_);
      if (core::TrafficLogic::isPositionValid(x_pos, traffic_lights_))
      {
        const double cycle_time = cycle_dist(random_engine_);
        const int state_val = state_dist(random_engine_);
        const auto state = static_cast<core::TrafficLightState>(state_val);

        traffic_lights_.push_back(core::TrafficLight::create(
            light_id++, core::Position{x_pos, 7.5}, 1U, state, cycle_time));
        break;
      }
    }
  }

  // Generate irrelevant lights (in lane 2 - not ego vehicle's lane)
  for (std::size_t i = 0; i < kNumIrrelevantLights; ++i)
  {
    for (int attempt = 0; attempt < 20; ++attempt)
    {
      const double x_pos = pos_dist(random_engine_);
      if (core::TrafficLogic::isPositionValid(x_pos, traffic_lights_))
      {
        const double cycle_time = cycle_dist(random_engine_);
        const int state_val = state_dist(random_engine_);
        const auto state = static_cast<core::TrafficLightState>(state_val);

        // Lane 2 lights are at y = 2.5 (different lane)
        traffic_lights_.push_back(core::TrafficLight::create(
            light_id++, core::Position{x_pos, 2.5}, 2U, state, cycle_time));
        break;
      }
    }
  }

  // Sort lights by x position for consistent display
  std::sort(traffic_lights_.begin(),
            traffic_lights_.end(),
            [](const auto& a, const auto& b) { return a.getX() < b.getX(); });
}

void MainWindow::updateTrafficLights()
{
  for (auto& light : traffic_lights_)
  {
    light.update(kDeltaTime);
  }
}

bool MainWindow::shouldStopForLight() const
{
  return core::TrafficLogic::shouldStopForLight(ego_vehicle_, traffic_lights_);
}

void MainWindow::updateTrafficLightDetections()
{
  constexpr double kDetectionRange = 100.0;

  std::vector<perception::TrafficLightDetectionResult> detections;
  const double vehicle_x = ego_vehicle_.getX();
  const core::LaneId vehicle_lane = ego_vehicle_.getLaneId();

  for (const auto& light : traffic_lights_)
  {
    if (!light.isValid())
    {
      continue;
    }

    const double light_x = light.getX();
    const double distance = light_x - vehicle_x;

    // Only detect lights ahead and within range
    if (distance > 0.0 && distance < kDetectionRange)
    {
      // Determine if light is in vehicle's lane (relevant)
      const bool is_relevant = (light.getLaneId() == vehicle_lane);
      const bool in_fov = true; // Lights are always visible when ahead

      detections.emplace_back(
          light.getId(), light.getState(), distance, is_relevant, in_fov);
    }
  }

  // Sort by distance (closest first)
  std::sort(detections.begin(),
            detections.end(),
            [](const auto& a, const auto& b)
            { return a.distance < b.distance; });

  // Update the overlay
  detection_overlay_->updateLightDetections(detections);

  // Update stop line information
  // Find the closest stop line (at traffic light position) in vehicle's lane
  double closest_stop_line_distance = -1.0;
  bool is_approaching = false;

  for (const auto& light : traffic_lights_)
  {
    if (light.getLaneId() != vehicle_lane)
    {
      continue;
    }

    const double stop_line_x = light.getX(); // Stop line is at light position
    const double distance = stop_line_x - vehicle_x;

    if (distance > 0.0 && distance < kDetectionRange)
    {
      if (closest_stop_line_distance < 0.0 ||
          distance < closest_stop_line_distance)
      {
        closest_stop_line_distance = distance;
        // Approaching if moving toward it
        is_approaching = (ego_vehicle_.getSpeed() > 0.1);
      }
    }
  }

  detection_overlay_->updateStopLine(closest_stop_line_distance,
                                     is_approaching);
}

bool MainWindow::shouldStopForSign()
{
  const bool was_waiting = stop_sign_state_.was_stopped;
  const bool should_stop = core::TrafficLogic::shouldStopForSign(
      ego_vehicle_, traffic_signs_, stop_sign_state_, kDeltaTime);

  // Log transitions
  if (should_stop && !was_waiting)
  {
    diagnostic_logs_->logWarning("Vehicle stopping at STOP sign");
  }
  else if (!should_stop && was_waiting)
  {
    diagnostic_logs_->logInfo("Stop sign wait complete - Vehicle resuming");
  }

  return should_stop;
}

void MainWindow::updateLaneChange()
{
  // Update lane change controller
  lane_change_controller_.update(ego_vehicle_, road_segment_, kDeltaTime);

  // Update turn signal visualization
  perspective_view_->updateTurnSignal(ego_vehicle_.getTurnSignal());
}

void MainWindow::requestLaneChange(core::LaneChangeDirection direction)
{
  if (!is_running_)
  {
    return;
  }

  // Calculate target lane for gap analysis
  const core::LaneId current_lane = ego_vehicle_.getLaneId();
  core::LaneId target_lane = current_lane;
  if (direction == core::LaneChangeDirection::Left)
  {
    target_lane = current_lane + 1U;
  }
  else if (direction == core::LaneChangeDirection::Right)
  {
    target_lane = (current_lane > 1U) ? current_lane - 1U : 1U;
  }

  // Check for pedestrians in the lane change path
  bool pedestrian_blocking = false;
  if (pedestrians_enabled_)
  {
    constexpr double kPedestrianCheckDistance = 15.0;
    const auto target_boundary = road_segment_.getLaneBoundary(target_lane);

    for (const auto& pedestrian : pedestrians_)
    {
      if (!pedestrian.isCrossing())
      {
        continue;
      }

      // Check if pedestrian is ahead within check distance
      const double distance_x = pedestrian.getX() - ego_vehicle_.getX();
      if (distance_x > 0.0 && distance_x < kPedestrianCheckDistance)
      {
        // Check if pedestrian is in or crossing into target lane
        if (target_boundary.has_value())
        {
          const double lane_center = target_boundary->centerY();
          const double lane_half_width = 1.5;
          if (std::abs(pedestrian.getY() - lane_center) < lane_half_width)
          {
            pedestrian_blocking = true;
            break;
          }
        }
      }
    }
  }

  if (pedestrian_blocking)
  {
    diagnostic_logs_->logWarning(
        "Lane change rejected - Pedestrian crossing in target lane");
    return;
  }

  // Perform gap analysis
  // Convert NPC vehicles to EgoVehicle format for gap analysis
  std::vector<core::EgoVehicle> gap_check_vehicles;
  for (const auto& npc : npc_vehicles_)
  {
    core::EgoVehicle vehicle(core::Coordinate{npc.getX(), npc.getY()},
                             npc.getLaneId());
    vehicle.setSpeed(npc.getSpeed());
    gap_check_vehicles.push_back(vehicle);
  }
  const auto gap_result = perception::GapAnalyzer::analyze(
      ego_vehicle_, target_lane, gap_check_vehicles);

  // Request lane change
  const bool accepted = lane_change_controller_.requestLaneChange(
      ego_vehicle_, direction, road_segment_, gap_result);

  if (accepted)
  {
    diagnostic_logs_->logInfo(
        QString("Lane change %1 initiated")
            .arg(direction == core::LaneChangeDirection::Left ? "LEFT"
                                                              : "RIGHT"));
  }
  else
  {
    // Check why it was rejected
    if (!gap_result.is_safe)
    {
      diagnostic_logs_->logWarning(
          QString("Lane change rejected - Gap not safe (front: %1m, rear: %2m)")
              .arg(gap_result.front_gap, 0, 'f', 1)
              .arg(gap_result.rear_gap, 0, 'f', 1));
    }
    else if (lane_change_controller_.isActive())
    {
      diagnostic_logs_->logWarning(
          "Lane change rejected - Already in progress");
    }
    else
    {
      diagnostic_logs_->logWarning(
          "Lane change rejected - Invalid target lane");
    }
  }
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
  switch (event->key())
  {
  case Qt::Key_Left:
  case Qt::Key_A:
    requestLaneChange(core::LaneChangeDirection::Left);
    break;

  case Qt::Key_Right:
  case Qt::Key_D:
    requestLaneChange(core::LaneChangeDirection::Right);
    break;

  case Qt::Key_Escape:
    // Cancel lane change
    if (lane_change_controller_.isActive())
    {
      lane_change_controller_.cancel(ego_vehicle_);
      diagnostic_logs_->logInfo("Lane change cancelled");
    }
    break;

  default:
    QMainWindow::keyPressEvent(event);
    break;
  }
}

void MainWindow::onNpcToggle(int state)
{
  npcs_enabled_ = (state == Qt::Checked);

  if (npcs_enabled_)
  {
    spawnNPCs();
    diagnostic_logs_->logInfo("NPCs enabled - spawned vehicles");
  }
  else
  {
    npc_vehicles_.clear();
    perspective_view_->updateNPCVehicles({});
    diagnostic_logs_->logInfo("NPCs disabled");
  }
}

void MainWindow::spawnNPCs()
{
  npc_vehicles_.clear();

  // Random distributions for spawn positions
  std::uniform_real_distribution<double> x_dist(10.0, 80.0);
  std::uniform_real_distribution<double> speed_dist(2.5, 5.5);
  std::uniform_int_distribution<core::LaneId> lane_dist(1U, 3U);

  constexpr double kMinSpawnDistance = 10.0; // Minimum distance between NPCs
  constexpr int kMaxAttempts = 20;           // Maximum spawn attempts per NPC

  // Spawn 4 NPCs at random positions, avoiding overlaps
  for (std::uint32_t id = 1U; id <= 4U; ++id)
  {
    bool valid_position = false;
    double x = 0.0;
    core::LaneId lane = 1U;

    for (int attempt = 0; attempt < kMaxAttempts && !valid_position; ++attempt)
    {
      x = x_dist(random_engine_);
      lane = lane_dist(random_engine_);

      // Check distance from existing NPCs in the same lane
      valid_position = true;
      for (const auto& existing_npc : npc_vehicles_)
      {
        if (existing_npc.getLaneId() == lane)
        {
          const double distance = std::abs(existing_npc.getX() - x);
          if (distance < kMinSpawnDistance)
          {
            valid_position = false;
            break;
          }
        }
      }
    }

    // Only add if valid position found (otherwise skip this NPC)
    if (valid_position)
    {
      const double speed = speed_dist(random_engine_);
      npc_vehicles_.emplace_back(id, x, lane, speed);
    }
  }

  // Set initial Y positions based on lane centers
  for (auto& npc : npc_vehicles_)
  {
    const auto boundary = road_segment_.getLaneBoundary(npc.getLaneId());
    if (boundary.has_value())
    {
      npc.setY(boundary->centerY());
    }
  }
}

void MainWindow::updateNPCs()
{
  if (!npcs_enabled_ || npc_vehicles_.empty())
  {
    return;
  }

  // Sort NPCs by X position for following distance calculation
  std::vector<std::pair<double, core::LaneId>> positions;
  positions.reserve(npc_vehicles_.size() + 1U);

  // Add ego vehicle to positions
  positions.emplace_back(ego_vehicle_.getX(), ego_vehicle_.getLaneId());

  for (const auto& npc : npc_vehicles_)
  {
    positions.emplace_back(npc.getX(), npc.getLaneId());
  }

  // Update each NPC
  for (auto& npc : npc_vehicles_)
  {
    // Find vehicle ahead in same lane
    double vehicle_ahead_x = -1.0;
    double min_distance = 1000.0;

    for (const auto& pos : positions)
    {
      if (pos.second == npc.getLaneId())
      {
        const double distance = pos.first - npc.getX();
        if (distance > 0.0 && distance < min_distance)
        {
          min_distance = distance;
          vehicle_ahead_x = pos.first;
        }
      }
    }

    npc.update(kDeltaTime,
               road_segment_,
               traffic_lights_,
               traffic_signs_,
               vehicle_ahead_x);

    // Respawn if NPC goes off screen
    if (npc.getX() > road_segment_.getEndX() + 20.0)
    {
      npc.reset(-30.0); // Respawn behind
    }
  }

  // Update visualization
  perspective_view_->updateNPCVehicles(npc_vehicles_);
}

void MainWindow::onPedestriansToggle(int state)
{
  pedestrians_enabled_ = (state == Qt::Checked);

  if (pedestrians_enabled_)
  {
    spawnPedestrians();
    diagnostic_logs_->logInfo("Pedestrians enabled");
  }
  else
  {
    pedestrians_.clear();
    perspective_view_->updatePedestrians({});
    diagnostic_logs_->logInfo("Pedestrians disabled");
  }
}

void MainWindow::spawnPedestrians()
{
  pedestrians_.clear();

  const double lane_min_y = -6.0; // Left side of road
  const double lane_max_y = 6.0;  // Right side of road

  // Random distributions for spawn positions
  std::uniform_real_distribution<double> x_dist(15.0, 85.0);
  std::uniform_real_distribution<double> speed_dist(0.7, 1.4);
  std::uniform_int_distribution<int> side_dist(0, 1);

  // Spawn 3 pedestrians at random positions
  for (std::uint32_t id = 1U; id <= 3U; ++id)
  {
    const double x = x_dist(random_engine_);
    const double speed = speed_dist(random_engine_);
    const bool from_left = (side_dist(random_engine_) == 0);
    const double start_y = from_left ? lane_min_y : lane_max_y;
    const double target_y = from_left ? lane_max_y : lane_min_y;
    pedestrians_.emplace_back(id, x, start_y, target_y, speed);
  }
}

void MainWindow::updatePedestrians()
{
  if (!pedestrians_enabled_ || pedestrians_.empty())
  {
    return;
  }

  for (auto& pedestrian : pedestrians_)
  {
    pedestrian.update(kDeltaTime);

    // Respawn completed pedestrians after a delay
    if (pedestrian.getState() == core::PedestrianState::Completed)
    {
      // Reset to opposite side
      const double current_y = pedestrian.getY();
      const double new_start = (current_y < 0.0) ? 6.0 : -6.0;
      const double new_target = (current_y < 0.0) ? -6.0 : 6.0;
      pedestrian.reset(pedestrian.getX(), new_start, new_target);
    }
  }

  // Update visualization
  perspective_view_->updatePedestrians(pedestrians_);
}

bool MainWindow::shouldEmergencyBrake() const
{
  if (!pedestrians_enabled_ || pedestrians_.empty())
  {
    return false;
  }

  constexpr double kEmergencyBrakeDistance = 5.0;
  constexpr double kLaneWidth = 2.5;

  for (const auto& pedestrian : pedestrians_)
  {
    // Only brake for crossing pedestrians
    if (!pedestrian.isCrossing())
    {
      continue;
    }

    // Check if pedestrian is ahead within braking distance
    const double distance_x = pedestrian.getX() - ego_vehicle_.getX();
    if (distance_x > 0.0 && distance_x < kEmergencyBrakeDistance)
    {
      // Check if pedestrian is in our lane (Y position overlap)
      const double ego_y = ego_vehicle_.getY();
      const double ped_y = pedestrian.getY();
      if (std::abs(ped_y - ego_y) < kLaneWidth)
      {
        return true;
      }
    }
  }

  return false;
}

} // namespace ui
} // namespace adas
