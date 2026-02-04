/**
 * @file test_ui_unit.cpp
 * @brief Unit tests for UI components (Headless)
 */

#include "detection_overlay.hpp"
#include "diagnostic_logs.hpp"
#include "ego_vehicle.hpp"
#include "perception_filter.hpp"
#include "perspective_view.hpp"
#include "road_segment.hpp"
#include "traffic_light.hpp"
#include "traffic_sign.hpp"

#include <QApplication>
#include <gtest/gtest.h>

using namespace adas::ui;
using namespace adas::core;
using namespace adas::perception;

class UiUnitTest : public ::testing::Test
{
protected:
  // widgets need valid qApp
};

// ===================================
// DiagnosticLogs Tests
// ===================================

TEST_F(UiUnitTest, DiagnosticLogsBasics)
{
  DiagnosticLogs logs;

  logs.logInfo("Info message");
  logs.logWarning("Warning message");

  QString content = logs.getLogContent();
  EXPECT_TRUE(content.contains("Info message"));
  EXPECT_TRUE(content.contains("Warning message"));

  // Actual format is "INFO" without brackets in prefix (brackets are around
  // timestamp)
  EXPECT_TRUE(content.contains("INFO"));
  EXPECT_TRUE(content.contains("WARN"));
}

TEST_F(UiUnitTest, DiagnosticLogsClear)
{
  DiagnosticLogs logs;
  logs.logError("Error");
  EXPECT_FALSE(logs.getLogContent().isEmpty());

  logs.clear();
  EXPECT_TRUE(logs.getLogContent().isEmpty());
}

TEST_F(UiUnitTest, DiagnosticLogsSafetyViolation)
{
  DiagnosticLogs logs;
  logs.logSafetyViolation("Collision!");

  QString content = logs.getLogContent();
  EXPECT_TRUE(content.contains("SAFETY"));
  EXPECT_TRUE(content.contains("Collision!"));
}

TEST_F(UiUnitTest, ValidationStateChange)
{
  DiagnosticLogs logs;
  logs.onStateChanged("System", "Off", "On");
  QString content = logs.getLogContent();
  EXPECT_TRUE(content.contains("System: Off"));
  EXPECT_TRUE(content.contains("On"));
}

// ===================================
// DetectionOverlay Tests
// ===================================

TEST_F(UiUnitTest, DetectionOverlayUpdates)
{
  DetectionOverlay overlay;

  // Create mock detections
  TrafficSign sign =
      TrafficSign::create(1, TrafficSignType::Stop, Position{100, 0}, 1);
  DetectionResult sign_res(sign, 50.0, true, true);
  std::vector<DetectionResult> sign_vec = {sign_res};

  overlay.updateDetections(sign_vec);

  // Traffic Lights
  TrafficLightDetectionResult light_res(
      1U, TrafficLightState::Red, 30.0, true, true);
  std::vector<TrafficLightDetectionResult> light_vec = {light_res};

  overlay.updateLightDetections(light_vec);

  // Lead Vehicle
  overlay.updateLeadVehicle(20.0, 15.0, 18.0);

  // Stop Line
  overlay.updateStopLine(10.0, true);

  overlay.clear();
}

// ===================================
// PerspectiveView Tests
// ===================================

TEST_F(UiUnitTest, PerspectiveViewUpdates)
{
  PerspectiveView view;

  EgoVehicle ego(Coordinate{0.0, 0.0, 0.0}, 1U);
  view.updateEgoVehicle(ego);

  RoadSegment road(1U, 0, 100);
  road.addLane(1U, LaneBoundary(4.0, 0.0));
  view.updateRoad(road);

  // Cycle views
  view.setViewMode(ViewMode::TopView);
  EXPECT_EQ(view.getViewMode(), ViewMode::TopView);
  view.cycleViewMode();
  EXPECT_NE(view.getViewMode(), ViewMode::TopView);

  // Clear
  view.clearScene();
}

TEST_F(UiUnitTest, PerspectiveViewSignsAndLights)
{
  PerspectiveView view;

  TrafficSign sign =
      TrafficSign::create(1, TrafficSignType::Yield, Position{50, 0}, 1);
  std::vector<TrafficSign> signs = {sign};
  view.updateSigns(signs, {1});

  TrafficLight light = TrafficLight::create(
      1, Position{100, 0}, 1, TrafficLightState::Green, 10.0);
  std::vector<TrafficLight> lights = {light};
  view.updateTrafficLights(lights, 1);
}

// ===================================
// MainWindow Tests
// ===================================

#include "main_window.hpp"

TEST_F(UiUnitTest, MainWindowLifecycle)
{
  MainWindow w;
  w.show();
  w.startSimulation();
  w.stopSimulation();
  w.resetSimulation();
}

#include <QKeyEvent>

TEST_F(UiUnitTest, LaneChangeInput)
{
  MainWindow w;
  w.show();
  w.startSimulation(); // Simulation must be running

  // Access logs to verify output
  auto *logs = w.findChild<DiagnosticLogs *>();
  ASSERT_NE(logs, nullptr);
  logs->clear();

  // Test 'A' key (Left Lane Change)
  QKeyEvent eventLeft(QEvent::KeyPress, Qt::Key_A, Qt::NoModifier);
  QApplication::sendEvent(&w, &eventLeft);

  QString contentLeft = logs->getLogContent();
  // Should log initiation or rejection (e.g. invalid lane)
  EXPECT_TRUE(contentLeft.contains("Lane change") ||
              contentLeft.contains("initiated") ||
              contentLeft.contains("rejected"))
      << "Log content was: " << contentLeft.toStdString();

  logs->clear();

  // Test 'D' key (Right Lane Change)
  QKeyEvent eventRight(QEvent::KeyPress, Qt::Key_D, Qt::NoModifier);
  QApplication::sendEvent(&w, &eventRight);

  QString contentRight = logs->getLogContent();
  EXPECT_TRUE(contentRight.contains("Lane change") ||
              contentRight.contains("initiated") ||
              contentRight.contains("rejected"))
      << "Log content was: " << contentRight.toStdString();
}

int main(int argc, char **argv)
{
  // Force offscreen to avoid X11/Wayland connection issues in test env
  qputenv("QT_QPA_PLATFORM", "offscreen");

  QApplication app(argc, argv);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
