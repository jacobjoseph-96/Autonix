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
      ego_vehicle_(core::Coordinate{0.0, 5.0, 0.0}, 1U),
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

  ego_vehicle_ = core::EgoVehicle(core::Coordinate{0.0, 5.0, 0.0}, 1U);
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
  perspective_view_->updateEgoVehicle(ego_vehicle_);
  detection_overlay_->clear();
  stop_sign_state_ = core::StopSignState{};

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
  const bool should_stop = should_stop_light || should_stop_sign;

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

  if (should_stop)
  {
    ego_vehicle_.setSpeed(0.0); // Stop for red light or stop sign
  }
  else
  {
    ego_vehicle_.setSpeed(5.0); // 5 m/s = 18 km/h
  }

  ego_vehicle_.update(kDeltaTime);

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

} // namespace ui
} // namespace adas
