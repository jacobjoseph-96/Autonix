//! @file detection_overlay.hpp
//! @brief High-contrast detection dashboard widget for signs and lights
//!

#ifndef ADAS_UI_DETECTION_OVERLAY_HPP
#define ADAS_UI_DETECTION_OVERLAY_HPP

#include "perception_filter.hpp"
#include "traffic_light.hpp"
#include "types.hpp"
#include <QFrame>
#include <QLabel>
#include <QString>
#include <QVBoxLayout>
#include <QWidget>
#include <memory>
#include <vector>

namespace adas
{
namespace ui
{

//! @brief Single sign detection entry display
//!
class SignDetectionEntry : public QFrame
{
  Q_OBJECT

public:
  explicit SignDetectionEntry(QWidget *parent = nullptr);

  void update(const perception::DetectionResult& result);
  void clear();

private:
  std::unique_ptr<QVBoxLayout> layout_;
  std::unique_ptr<QLabel> type_label_;
  std::unique_ptr<QLabel> distance_label_;
  std::unique_ptr<QLabel> relevance_label_;
};

//! @brief Single traffic light detection entry display
//!
class LightDetectionEntry : public QFrame
{
  Q_OBJECT

public:
  explicit LightDetectionEntry(QWidget *parent = nullptr);

  void update(const perception::TrafficLightDetectionResult& result);
  void clear();

private:
  std::unique_ptr<QVBoxLayout> layout_;
  std::unique_ptr<QLabel> state_label_;
  std::unique_ptr<QLabel> distance_label_;
  std::unique_ptr<QLabel> relevance_label_;
};

//! @brief High-contrast dashboard for detection overlay
//!
class DetectionOverlay : public QWidget
{
  Q_OBJECT

public:
  explicit DetectionOverlay(QWidget *parent = nullptr);
  ~DetectionOverlay() override = default;

  //! @brief Update displayed sign detections
  //! @param detections Detection results from perception filter
  //!
  void
  updateDetections(const std::vector<perception::DetectionResult>& detections);

  //! @brief Update displayed traffic light detections
  //! @param detections Traffic light detection results
  //!
  void updateLightDetections(
      const std::vector<perception::TrafficLightDetectionResult>& detections);

  //! @brief Clear all detection displays
  //!
  void clear();

  //! @brief Update stop line information
  //! @param distance_to_next Distance to next stop line in meters
  //! @param is_approaching Whether vehicle is approaching stop line
  //!
  void updateStopLine(double distance_to_next, bool is_approaching);

  //! @brief Update lead vehicle information
  //! @param distance Distance to lead vehicle in meters (-1 if no lead)
  //! @param lead_speed Speed of lead vehicle in m/s
  //! @param ego_speed Speed of ego vehicle in m/s
  //!
  void updateLeadVehicle(double distance, double lead_speed, double ego_speed);

  //! @brief Set maximum number of displayed detections
  //!
  void setMaxDisplayed(std::size_t max_count);

signals:
  //! @brief Emitted when a relevant sign is detected
  //!
  void relevantSignDetected(const QString& type, double distance);

  //! @brief Emitted when a relevant traffic light is detected
  //!
  void relevantLightDetected(const QString& state, double distance);

private:
  static constexpr std::size_t kDefaultMaxDisplayed = 5U;

  std::unique_ptr<QVBoxLayout> main_layout_;

  // Signs section
  std::unique_ptr<QLabel> signs_header_label_;
  std::unique_ptr<QLabel> no_signs_label_;
  std::vector<std::unique_ptr<SignDetectionEntry>> sign_entries_;

  // Lights section
  std::unique_ptr<QLabel> lights_header_label_;
  std::unique_ptr<QLabel> no_lights_label_;
  std::vector<std::unique_ptr<LightDetectionEntry>> light_entries_;

  // Stop line section
  std::unique_ptr<QLabel> stop_line_header_label_;
  std::unique_ptr<QLabel> stop_line_distance_label_;
  std::unique_ptr<QLabel> stop_line_status_label_;

  // Lead vehicle section
  std::unique_ptr<QLabel> lead_vehicle_header_label_;
  std::unique_ptr<QLabel> ego_speed_label_;
  std::unique_ptr<QLabel> lead_vehicle_distance_label_;
  std::unique_ptr<QLabel> lead_vehicle_speed_label_;

  std::size_t max_displayed_{kDefaultMaxDisplayed};

  void setupUi();
  void ensureSignEntryCount(std::size_t count);
  void ensureLightEntryCount(std::size_t count);
};

// Keep old name for backwards compatibility
using DetectionEntry = SignDetectionEntry;

} // namespace ui
} // namespace adas

#endif // ADAS_UI_DETECTION_OVERLAY_HPP
