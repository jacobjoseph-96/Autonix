/**
 * @file detection_overlay.cpp
 * @brief Detection overlay and entry implementation
 */

#include "detection_overlay.hpp"
#include <QFont>

namespace adas
{
namespace ui
{

// =============================================================================
// SignDetectionEntry Implementation
// =============================================================================

SignDetectionEntry::SignDetectionEntry(QWidget *parent)
    : QFrame(parent), layout_(std::make_unique<QVBoxLayout>(this)),
      type_label_(std::make_unique<QLabel>()),
      distance_label_(std::make_unique<QLabel>()),
      relevance_label_(std::make_unique<QLabel>())
{
  setFrameStyle(QFrame::StyledPanel | QFrame::Raised);
  setStyleSheet("SignDetectionEntry {"
                "  background-color: #2d2d44;"
                "  border-radius: 8px;"
                "  padding: 10px;"
                "  margin: 4px;"
                "}");

  QFont type_font;
  type_font.setPointSize(14);
  type_font.setBold(true);
  type_label_->setFont(type_font);
  type_label_->setStyleSheet("color: #ffffff;");

  QFont data_font;
  data_font.setPointSize(11);
  distance_label_->setFont(data_font);
  distance_label_->setStyleSheet("color: #b0b0b0;");

  relevance_label_->setFont(data_font);

  layout_->setSpacing(4);
  layout_->addWidget(type_label_.get());
  layout_->addWidget(distance_label_.get());
  layout_->addWidget(relevance_label_.get());

  clear();
}

void SignDetectionEntry::update(const perception::DetectionResult& result)
{
  type_label_->setText(QString("🚸 %1").arg(result.sign.getTypeString()));
  distance_label_->setText(
      QString("Distance: %1 m").arg(result.distance, 0, 'f', 2));

  if (result.is_relevant)
  {
    relevance_label_->setText("✓ RELEVANT");
    relevance_label_->setStyleSheet("color: #4caf50; font-weight: bold;");
    setStyleSheet("SignDetectionEntry {"
                  "  background-color: #1b3d1b;"
                  "  border: 2px solid #4caf50;"
                  "  border-radius: 8px;"
                  "  padding: 10px;"
                  "  margin: 4px;"
                  "}");
  }
  else
  {
    relevance_label_->setText("○ Not Relevant");
    relevance_label_->setStyleSheet("color: #757575;");
    setStyleSheet("SignDetectionEntry {"
                  "  background-color: #2d2d44;"
                  "  border-radius: 8px;"
                  "  padding: 10px;"
                  "  margin: 4px;"
                  "}");
  }

  setVisible(true);
}

void SignDetectionEntry::clear()
{
  type_label_->setText("--");
  distance_label_->setText("Distance: -- m");
  relevance_label_->setText("--");
  relevance_label_->setStyleSheet("color: #757575;");
  setVisible(false);
}

// =============================================================================
// LightDetectionEntry Implementation
// =============================================================================

LightDetectionEntry::LightDetectionEntry(QWidget *parent)
    : QFrame(parent), layout_(std::make_unique<QVBoxLayout>(this)),
      state_label_(std::make_unique<QLabel>()),
      distance_label_(std::make_unique<QLabel>()),
      relevance_label_(std::make_unique<QLabel>())
{
  setFrameStyle(QFrame::StyledPanel | QFrame::Raised);
  setStyleSheet("LightDetectionEntry {"
                "  background-color: #2d2d44;"
                "  border-radius: 8px;"
                "  padding: 10px;"
                "  margin: 4px;"
                "}");

  QFont state_font;
  state_font.setPointSize(14);
  state_font.setBold(true);
  state_label_->setFont(state_font);
  state_label_->setStyleSheet("color: #ffffff;");

  QFont data_font;
  data_font.setPointSize(11);
  distance_label_->setFont(data_font);
  distance_label_->setStyleSheet("color: #b0b0b0;");

  relevance_label_->setFont(data_font);

  layout_->setSpacing(4);
  layout_->addWidget(state_label_.get());
  layout_->addWidget(distance_label_.get());
  layout_->addWidget(relevance_label_.get());

  clear();
}

void LightDetectionEntry::update(
    const perception::TrafficLightDetectionResult& result)
{
  // Set icon and color based on state
  QString state_text;
  QString state_color;

  switch (result.state)
  {
  case core::TrafficLightState::Red:
    state_text = "🔴 RED";
    state_color = "#f44336";
    break;
  case core::TrafficLightState::Yellow:
    state_text = "🟡 YELLOW";
    state_color = "#ffeb3b";
    break;
  case core::TrafficLightState::Green:
    state_text = "🟢 GREEN";
    state_color = "#4caf50";
    break;
  }

  state_label_->setText(state_text);
  state_label_->setStyleSheet(QString("color: %1;").arg(state_color));

  distance_label_->setText(
      QString("Distance: %1 m").arg(result.distance, 0, 'f', 2));

  if (result.is_relevant)
  {
    relevance_label_->setText("✓ RELEVANT");
    relevance_label_->setStyleSheet("color: #4caf50; font-weight: bold;");

    // Color background based on light state for relevant lights
    QString border_color;
    QString bg_color;
    switch (result.state)
    {
    case core::TrafficLightState::Red:
      border_color = "#f44336";
      bg_color = "#3d1b1b";
      break;
    case core::TrafficLightState::Yellow:
      border_color = "#ffeb3b";
      bg_color = "#3d3d1b";
      break;
    case core::TrafficLightState::Green:
      border_color = "#4caf50";
      bg_color = "#1b3d1b";
      break;
    }

    setStyleSheet(QString("LightDetectionEntry {"
                          "  background-color: %1;"
                          "  border: 2px solid %2;"
                          "  border-radius: 8px;"
                          "  padding: 10px;"
                          "  margin: 4px;"
                          "}")
                      .arg(bg_color, border_color));
  }
  else
  {
    relevance_label_->setText("○ Not Relevant");
    relevance_label_->setStyleSheet("color: #757575;");
    setStyleSheet("LightDetectionEntry {"
                  "  background-color: #2d2d44;"
                  "  border-radius: 8px;"
                  "  padding: 10px;"
                  "  margin: 4px;"
                  "}");
  }

  setVisible(true);
}

void LightDetectionEntry::clear()
{
  state_label_->setText("--");
  state_label_->setStyleSheet("color: #757575;");
  distance_label_->setText("Distance: -- m");
  relevance_label_->setText("--");
  relevance_label_->setStyleSheet("color: #757575;");
  setVisible(false);
}

// =============================================================================
// DetectionOverlay Implementation
// =============================================================================

DetectionOverlay::DetectionOverlay(QWidget *parent)
    : QWidget(parent), main_layout_(std::make_unique<QVBoxLayout>(this)),
      signs_header_label_(std::make_unique<QLabel>("🚸 TRAFFIC SIGNS")),
      no_signs_label_(std::make_unique<QLabel>("No signs detected")),
      lights_header_label_(std::make_unique<QLabel>("🚦 TRAFFIC LIGHTS")),
      no_lights_label_(std::make_unique<QLabel>("No lights detected")),
      stop_line_header_label_(std::make_unique<QLabel>("🛑 STOP LINE")),
      stop_line_distance_label_(std::make_unique<QLabel>("Distance: -- m")),
      stop_line_status_label_(std::make_unique<QLabel>("No stop line ahead"))
{
  setupUi();
}

void DetectionOverlay::setupUi()
{
  setStyleSheet("DetectionOverlay {"
                "  background-color: #1a1a2e;"
                "}");

  QFont header_font;
  header_font.setPointSize(12);
  header_font.setBold(true);

  // Signs section header
  signs_header_label_->setFont(header_font);
  signs_header_label_->setStyleSheet("color: #ffffff;"
                                     "padding: 10px;"
                                     "background-color: #16213e;"
                                     "border-radius: 4px;");
  signs_header_label_->setAlignment(Qt::AlignCenter);

  no_signs_label_->setStyleSheet("color: #666; padding: 10px;");
  no_signs_label_->setAlignment(Qt::AlignCenter);

  // Lights section header
  lights_header_label_->setFont(header_font);
  lights_header_label_->setStyleSheet("color: #ffffff;"
                                      "padding: 10px;"
                                      "background-color: #16213e;"
                                      "border-radius: 4px;");
  lights_header_label_->setAlignment(Qt::AlignCenter);

  no_lights_label_->setStyleSheet("color: #666; padding: 10px;");
  no_lights_label_->setAlignment(Qt::AlignCenter);

  main_layout_->setContentsMargins(8, 8, 8, 8);
  main_layout_->setSpacing(8);

  // Add signs section
  main_layout_->addWidget(signs_header_label_.get());
  main_layout_->addWidget(no_signs_label_.get());

  // Add separator between signs and lights
  auto *separator1 = new QFrame();
  separator1->setFrameShape(QFrame::HLine);
  separator1->setStyleSheet("background-color: #333; margin: 5px 0;");
  main_layout_->addWidget(separator1);

  // Add lights section
  main_layout_->addWidget(lights_header_label_.get());
  main_layout_->addWidget(no_lights_label_.get());

  // Add separator between lights and stop line
  auto *separator2 = new QFrame();
  separator2->setFrameShape(QFrame::HLine);
  separator2->setStyleSheet("background-color: #333; margin: 5px 0;");
  main_layout_->addWidget(separator2);

  // Add stop line section
  stop_line_header_label_->setFont(header_font);
  stop_line_header_label_->setStyleSheet("color: #ffffff;"
                                         "padding: 10px;"
                                         "background-color: #16213e;"
                                         "border-radius: 4px;");
  stop_line_header_label_->setAlignment(Qt::AlignCenter);

  QFont data_font;
  data_font.setPointSize(11);
  stop_line_distance_label_->setFont(data_font);
  stop_line_distance_label_->setStyleSheet("color: #b0b0b0; padding: 5px;");
  stop_line_distance_label_->setAlignment(Qt::AlignCenter);

  stop_line_status_label_->setFont(data_font);
  stop_line_status_label_->setStyleSheet("color: #757575; padding: 5px;");
  stop_line_status_label_->setAlignment(Qt::AlignCenter);

  main_layout_->addWidget(stop_line_header_label_.get());
  main_layout_->addWidget(stop_line_distance_label_.get());
  main_layout_->addWidget(stop_line_status_label_.get());

  main_layout_->addStretch();

  // Pre-create entry widgets
  ensureSignEntryCount(kDefaultMaxDisplayed);
  ensureLightEntryCount(kDefaultMaxDisplayed);
}

void DetectionOverlay::ensureSignEntryCount(std::size_t count)
{
  while (sign_entries_.size() < count)
  {
    auto entry = std::make_unique<SignDetectionEntry>(this);
    // Insert after signs_header_label (0), no_signs_label (1)
    // Position 2 + existing entries
    const int insert_pos = 2 + static_cast<int>(sign_entries_.size());
    main_layout_->insertWidget(insert_pos, entry.get());
    sign_entries_.push_back(std::move(entry));
  }
}

void DetectionOverlay::ensureLightEntryCount(std::size_t count)
{
  while (light_entries_.size() < count)
  {
    auto entry = std::make_unique<LightDetectionEntry>(this);
    // Layout order:
    // 0: signs_header, 1: no_signs, 2-6: sign entries (5), 7: separator1
    // 8: lights_header, 9: no_lights, then light entries, then separator2, stop
    // line... Insert after no_lights_label = position after lights_header + 1 +
    // existing light entries Base position = 2 + max_displayed (signs) + 1
    // (separator) + 2 (lights header + no_lights)
    const int base_pos = 2 + static_cast<int>(kDefaultMaxDisplayed) + 1 + 2;
    const int insert_pos = base_pos + static_cast<int>(light_entries_.size());
    main_layout_->insertWidget(insert_pos, entry.get());
    light_entries_.push_back(std::move(entry));
  }
}

void DetectionOverlay::updateDetections(
    const std::vector<perception::DetectionResult>& detections)
{
  const std::size_t display_count = std::min(detections.size(), max_displayed_);

  ensureSignEntryCount(display_count);

  if (detections.empty())
  {
    no_signs_label_->setVisible(true);
    for (auto& entry : sign_entries_)
    {
      entry->clear();
    }
    return;
  }

  no_signs_label_->setVisible(false);

  for (std::size_t i = 0; i < sign_entries_.size(); ++i)
  {
    if (i < display_count)
    {
      sign_entries_[i]->update(detections[i]);

      // Emit signal for relevant signs
      if (detections[i].is_relevant)
      {
        emit relevantSignDetected(QString(detections[i].sign.getTypeString()),
                                  detections[i].distance);
      }
    }
    else
    {
      sign_entries_[i]->clear();
    }
  }
}

void DetectionOverlay::updateLightDetections(
    const std::vector<perception::TrafficLightDetectionResult>& detections)
{
  const std::size_t display_count = std::min(detections.size(), max_displayed_);

  ensureLightEntryCount(display_count);

  if (detections.empty())
  {
    no_lights_label_->setVisible(true);
    for (auto& entry : light_entries_)
    {
      entry->clear();
    }
    return;
  }

  no_lights_label_->setVisible(false);

  for (std::size_t i = 0; i < light_entries_.size(); ++i)
  {
    if (i < display_count)
    {
      light_entries_[i]->update(detections[i]);

      // Emit signal for relevant lights
      if (detections[i].is_relevant)
      {
        emit relevantLightDetected(
            QString(core::trafficLightStateToString(detections[i].state)),
            detections[i].distance);
      }
    }
    else
    {
      light_entries_[i]->clear();
    }
  }
}

void DetectionOverlay::clear()
{
  no_signs_label_->setVisible(true);
  for (auto& entry : sign_entries_)
  {
    entry->clear();
  }

  no_lights_label_->setVisible(true);
  for (auto& entry : light_entries_)
  {
    entry->clear();
  }
}

void DetectionOverlay::setMaxDisplayed(std::size_t max_count)
{
  max_displayed_ = max_count;
  ensureSignEntryCount(max_count);
  ensureLightEntryCount(max_count);
}

void DetectionOverlay::updateStopLine(double distance_to_next,
                                      bool is_approaching)
{
  if (distance_to_next > 0.0 && distance_to_next < 100.0)
  {
    // There's a stop line ahead
    stop_line_distance_label_->setText(
        QString("Distance: %1 m").arg(distance_to_next, 0, 'f', 1));

    if (is_approaching && distance_to_next < 10.0)
    {
      // Close to stop line
      stop_line_status_label_->setText("⚠️ APPROACHING STOP LINE");
      stop_line_status_label_->setStyleSheet(
          "color: #ffeb3b; font-weight: bold; padding: 5px;");
    }
    else if (is_approaching)
    {
      stop_line_status_label_->setText("→ Stop line ahead");
      stop_line_status_label_->setStyleSheet("color: #4caf50; padding: 5px;");
    }
    else
    {
      stop_line_status_label_->setText("○ Stop line detected");
      stop_line_status_label_->setStyleSheet("color: #b0b0b0; padding: 5px;");
    }
  }
  else
  {
    // No stop line ahead
    stop_line_distance_label_->setText("Distance: -- m");
    stop_line_status_label_->setText("No stop line ahead");
    stop_line_status_label_->setStyleSheet("color: #757575; padding: 5px;");
  }
}

} // namespace ui
} // namespace adas
