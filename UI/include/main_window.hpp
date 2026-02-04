//! @file main_window.hpp
//! @brief Main application window with dock widgets
//!

#ifndef ADAS_UI_MAIN_WINDOW_HPP
#define ADAS_UI_MAIN_WINDOW_HPP

#include "traffic_logic.hpp"
#include <QCheckBox>
#include <QComboBox>
#include <QDockWidget>
#include <QKeyEvent>
#include <QMainWindow>
#include <QSpinBox>
#include <QTimer>
#include <QToolBar>
#include <memory>
#include <random>
#include <set>
#include <vector>

#include "adaptive_cruise_controller.hpp"
#include "detection_overlay.hpp"
#include "diagnostic_logs.hpp"
#include "ego_vehicle.hpp"
#include "gap_analyzer.hpp"
#include "lane_change_controller.hpp"
#include "npc_vehicle.hpp"
#include "pedestrian.hpp"
#include "perception_filter.hpp"
#include "perspective_view.hpp"
#include "road_segment.hpp"
#include "traffic_light.hpp"
#include "traffic_sign.hpp"

namespace adas
{
namespace ui
{

//! @brief Main application window with three dock components
//!
class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = nullptr);
  ~MainWindow() override = default;

  //! @brief Start the simulation loop
  //!
  void startSimulation();

  //! @brief Stop the simulation loop
  //!
  void stopSimulation();

  //! @brief Reset simulation to initial state
  //!
  void resetSimulation();

private slots:
  //! @brief Simulation update tick
  //!
  void onSimulationTick();

  //! @brief Handle ACC toggle checkbox
  //!
  void onAccToggle(int state);

  //! @brief Handle ACC mode change
  //!
  void onAccModeChanged(int index);

  //! @brief Handle ACC speed change
  //!
  void onAccSpeedChanged(int value);

  //! @brief Handle NPC toggle checkbox
  //!
  void onNpcToggle(int state);

  //! @brief Handle Pedestrians toggle checkbox
  //!
  void onPedestriansToggle(int state);

private:
  // UI Components (unique_ptr for smart pointer management)
  std::unique_ptr<QDockWidget> perspective_dock_;
  std::unique_ptr<QDockWidget> overlay_dock_;
  std::unique_ptr<QDockWidget> logs_dock_;

  std::unique_ptr<PerspectiveView> perspective_view_;
  std::unique_ptr<DetectionOverlay> detection_overlay_;
  std::unique_ptr<DiagnosticLogs> diagnostic_logs_;

  // Simulation timer
  std::unique_ptr<QTimer> simulation_timer_;

  // World model
  core::EgoVehicle ego_vehicle_;
  core::RoadSegment road_segment_;
  std::vector<core::TrafficSign> traffic_signs_;
  std::vector<core::TrafficLight> traffic_lights_;

  // Perception
  perception::PerceptionFilter perception_filter_;

  // Lane change controller
  core::LaneChangeController lane_change_controller_;

  // Adaptive Cruise Controller
  std::unique_ptr<core::AdaptiveCruiseController> acc_controller_;

  // NPC vehicles
  std::vector<core::NPCVehicle> npc_vehicles_;
  bool npcs_enabled_{false};
  QCheckBox *npc_toggle_{nullptr};

  // Random engine (MISRA compliant - no rand())
  std::mt19937 random_engine_;

  // Simulation state
  bool is_running_{false};
  double simulation_time_{0.0};
  bool was_stopped_for_light_{false};
  core::StopSignState stop_sign_state_;

  static constexpr double kFrameRate = 60.0;
  static constexpr double kDeltaTime = 1.0 / kFrameRate;
  static constexpr double kStopSignWaitTime =
      0.5; // Wait 0.5 seconds at stop sign

  void setupUi();
  void setupToolBar();
  void setupDockWidgets();
  void setupSimulation();
  void initializeWorld();
  void generateTrafficSigns();
  void generateTrafficLights();
  void spawnNPCs();
  void updateNPCs();
  void spawnPedestrians();
  void updatePedestrians();
  void updatePerception();
  void updateTrafficLights();
  void updateTrafficLightDetections();
  void updateLaneChange();
  void requestLaneChange(core::LaneChangeDirection direction);
  [[nodiscard]] bool shouldStopForLight() const;
  [[nodiscard]] bool shouldStopForSign();
  [[nodiscard]] bool shouldEmergencyBrake() const;
  [[nodiscard]] double getDistanceToStopLine() const;

  // Toolbar
  std::unique_ptr<QToolBar> tool_bar_;

  // Pedestrians
  std::vector<core::Pedestrian> pedestrians_;
  bool pedestrians_enabled_{false};
  QCheckBox *pedestrians_toggle_{nullptr};

  // ACC UI controls
  QCheckBox *acc_toggle_{nullptr};
  QComboBox *acc_mode_selector_{nullptr};
  QSpinBox *acc_speed_selector_{nullptr};

protected:
  //! @brief Handle key press events for lane change input
  //!
  void keyPressEvent(QKeyEvent *event) override;
};

} // namespace ui
} // namespace adas

#endif // ADAS_UI_MAIN_WINDOW_HPP
