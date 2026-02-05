/**
 * @file perspective_view.cpp
 * @brief Perspective view implementation
 */

#include "perspective_view.hpp"
#include <QResizeEvent>
#include <QWheelEvent>
#include <QtMath>
#include <algorithm>

namespace adas
{
namespace ui
{

PerspectiveView::PerspectiveView(QWidget *parent)
    : QGraphicsView(parent), scene_(std::make_unique<QGraphicsScene>(this))
{
  setupScene();
}

void PerspectiveView::setupScene()
{
  setScene(scene_.get());
  setRenderHint(QPainter::Antialiasing);
  setViewportUpdateMode(QGraphicsView::FullViewportUpdate);

  // Enable scrollbars for manual panning
  setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
  setTransformationAnchor(QGraphicsView::AnchorViewCenter);
  setResizeAnchor(QGraphicsView::AnchorViewCenter);

  setStyleSheet("PerspectiveView {"
                "  background-color: #0f0f1a;"
                "  border: none;"
                "}");

  // Set scene rect for vertical road (swapped dimensions)
  // X: lateral position (narrow), Y: forward position (long road)
  scene_->setSceneRect(-50, -800, 100, 1000);

  // Create initial vehicle item
  vehicle_item_ = scene_->addPolygon(createVehiclePolygon(),
                                     QPen(Qt::white, 0.2),
                                     QBrush(QColor(66, 165, 245)) // Blue car
  );
  vehicle_item_->setZValue(100); // On top

  // Create FoV visualization
  fov_item_ = scene_->addPolygon(QPolygonF(),
                                 QPen(QColor(255, 255, 0, 80), 0.1),
                                 QBrush(QColor(255, 255, 0, 30)));
  fov_item_->setZValue(50);
}

void PerspectiveView::updateEgoVehicle(const core::EgoVehicle& vehicle)
{
  if (vehicle_item_ == nullptr)
  {
    return;
  }

  // Store vehicle position for view modes
  last_vehicle_x_ = vehicle.getX();
  last_vehicle_y_ = vehicle.getY();
  last_vehicle_heading_ = vehicle.getHeading();

  // For vertical road: swap x/y so road goes up (vehicle.x becomes scene y)
  const double scene_x = vehicle.getY() * scale_factor_; // Lateral position
  const double scene_y =
      -vehicle.getX() * scale_factor_; // Forward position (inverted for screen)

  // Transform vehicle polygon (arrow points upward by default)
  QTransform transform;
  transform.translate(scene_x, scene_y);
  // Apply vehicle heading rotation
  transform.rotateRadians(-vehicle.getHeading());

  vehicle_item_->setTransform(transform);

  // Apply view mode - always center on vehicle
  resetTransform();

  double view_scale = zoom_level_;
  switch (view_mode_)
  {
  case ViewMode::TopView:
    view_scale *= 2.0; // Medium zoom, centered on car
    break;
  case ViewMode::FrontView:
    view_scale *= 3.0; // Closer zoom, focused ahead
    break;
  case ViewMode::BirdEye:
    view_scale *= 0.8; // Wide view, see more of the road
    break;
  }

  scale(view_scale, view_scale);
  centerOn(scene_x, scene_y);
}

void PerspectiveView::updateRoad(const core::RoadSegment& road)
{
  // Clear existing lane items
  for (auto *item : lane_items_)
  {
    scene_->removeItem(item);
    delete item;
  }
  lane_items_.clear();

  const auto lane_ids = road.getLaneIds();

  for (const auto& lane_id : lane_ids)
  {
    const auto boundary = road.getLaneBoundary(lane_id);
    if (!boundary.has_value())
    {
      continue;
    }

    const double x_start = road.getStartX();
    const double x_end = road.getEndX();
    const double y_left = boundary->left_edge_y;
    const double y_right = boundary->right_edge_y;

    // For vertical road: swap x/y coordinates
    // Road goes vertical (up the screen), lanes are horizontal
    const double scene_left = y_right * scale_factor_;
    const double scene_right = y_left * scale_factor_;
    const double scene_top = -x_end * scale_factor_;      // Far end (top)
    const double scene_bottom = -x_start * scale_factor_; // Near end (bottom)

    auto *lane_rect = scene_->addRect(scene_left,
                                      scene_top,
                                      scene_right - scene_left,
                                      scene_bottom - scene_top,
                                      QPen(QColor(100, 100, 100), 0.2),
                                      QBrush(QColor(40, 40, 50)));
    lane_rect->setZValue(0);
    lane_items_.push_back(lane_rect);

    // Add lane center line
    const double center_x = (scene_left + scene_right) / 2.0;
    QPen dashed_pen(QColor(200, 200, 200), 0.3, Qt::DashLine);
    auto *center_line = scene_->addLine(
        center_x, scene_top, center_x, scene_bottom, dashed_pen);
    center_line->setZValue(1);
    lane_items_.push_back(static_cast<QGraphicsRectItem *>(
        dynamic_cast<QGraphicsItem *>(center_line)));
  }
}

void PerspectiveView::updateSigns(const std::vector<core::TrafficSign>& signs,
                                  const std::vector<core::SignId>& detected_ids)
{
  // Clear existing sign items
  for (auto *item : sign_items_)
  {
    scene_->removeItem(item);
    delete item;
  }
  sign_items_.clear();

  for (const auto& sign : signs)
  {
    if (!sign.isValid())
    {
      continue;
    }

    // For vertical road: swap x/y coordinates
    const double scene_x = sign.getPosition().y * scale_factor_;  // Lateral
    const double scene_y = -sign.getPosition().x * scale_factor_; // Forward
    const double size = 3.0; // Sign size in pixels

    const bool is_detected =
        std::find(detected_ids.begin(), detected_ids.end(), sign.getId()) !=
        detected_ids.end();

    QColor sign_color = getSignColor(sign.getType());
    if (is_detected)
    {
      sign_color = sign_color.lighter(130);
    }

    // Draw stop line for stop signs (left half only, same as traffic lights)
    if (sign.getType() == core::TrafficSignType::Stop)
    {
      constexpr double kLaneWidth = 5.0;
      const double half_lane = (kLaneWidth / 2.0) * scale_factor_;

      auto *stop_line =
          scene_->addLine(scene_x - half_lane, // Left edge
                          scene_y,
                          scene_x, // Center (only left half)
                          scene_y,
                          QPen(QColor(255, 255, 255), 1.5, Qt::SolidLine));
      stop_line->setZValue(50);
      sign_items_.push_back(stop_line);
    }

    // Draw sign as a circle
    auto *sign_item = scene_->addEllipse(scene_x - size / 2,
                                         scene_y - size / 2,
                                         size,
                                         size,
                                         QPen(Qt::white, 0.3),
                                         QBrush(sign_color));
    sign_item->setZValue(80);

    // Add sign type label
    QString label;
    switch (sign.getType())
    {
    case core::TrafficSignType::Stop:
      label = "S";
      break;
    case core::TrafficSignType::Yield:
      label = "Y";
      break;
    case core::TrafficSignType::SpeedLimit:
      label = "L";
      break;
    }

    auto *text_item = scene_->addText(label);
    text_item->setPos(scene_x - 2, scene_y - 4);
    text_item->setDefaultTextColor(Qt::white);
    text_item->setScale(0.3);
    text_item->setZValue(81);

    sign_items_.push_back(sign_item);
    sign_items_.push_back(text_item);
  }
}

void PerspectiveView::updateTrafficLights(
    const std::vector<core::TrafficLight>& lights,
    core::LaneId /*vehicle_lane_id*/)
{
  // Clear existing traffic light items
  for (auto *item : traffic_light_items_)
  {
    scene_->removeItem(item);
    delete item;
  }
  traffic_light_items_.clear();

  for (const auto& light : lights)
  {
    if (!light.isValid())
    {
      continue;
    }

    // For vertical road: swap x/y coordinates
    const double scene_x = light.getY() * scale_factor_;  // Lateral position
    const double scene_y = -light.getX() * scale_factor_; // Forward position

    // Stop line (white line on left half of lane only, for all traffic lights)
    constexpr double kLaneWidth = 5.0; // Width of lane in meters
    const double half_lane = (kLaneWidth / 2.0) * scale_factor_;

    // Draw only left half of stop line (from left edge to center)
    auto *stop_line =
        scene_->addLine(scene_x - half_lane, // Left edge
                        scene_y,
                        scene_x, // Center (only left half)
                        scene_y,
                        QPen(QColor(255, 255, 255), 1.5, Qt::SolidLine));
    stop_line->setZValue(50);
    traffic_light_items_.push_back(stop_line);

    // Traffic light housing (dark rectangle)
    const double housing_width = 4.0;
    const double housing_height = 10.0;
    auto *housing = scene_->addRect(scene_x - housing_width / 2,
                                    scene_y - housing_height / 2,
                                    housing_width,
                                    housing_height,
                                    QPen(Qt::white, 0.2),
                                    QBrush(QColor(30, 30, 30)));
    housing->setZValue(85);
    traffic_light_items_.push_back(housing);

    // Determine light colors based on current state
    QColor red_color = (light.getState() == core::TrafficLightState::Red)
                           ? QColor(255, 0, 0)
                           : QColor(80, 0, 0);
    QColor yellow_color = (light.getState() == core::TrafficLightState::Yellow)
                              ? QColor(255, 255, 0)
                              : QColor(80, 80, 0);
    QColor green_color = (light.getState() == core::TrafficLightState::Green)
                             ? QColor(0, 255, 0)
                             : QColor(0, 80, 0);

    const double light_size = 2.5;
    const double spacing = 3.0;

    // Red light (top)
    auto *red_light = scene_->addEllipse(scene_x - light_size / 2,
                                         scene_y - spacing - light_size / 2,
                                         light_size,
                                         light_size,
                                         QPen(Qt::NoPen),
                                         QBrush(red_color));
    red_light->setZValue(86);
    traffic_light_items_.push_back(red_light);

    // Yellow light (middle)
    auto *yellow_light = scene_->addEllipse(scene_x - light_size / 2,
                                            scene_y - light_size / 2,
                                            light_size,
                                            light_size,
                                            QPen(Qt::NoPen),
                                            QBrush(yellow_color));
    yellow_light->setZValue(86);
    traffic_light_items_.push_back(yellow_light);

    // Green light (bottom)
    auto *green_light = scene_->addEllipse(scene_x - light_size / 2,
                                           scene_y + spacing - light_size / 2,
                                           light_size,
                                           light_size,
                                           QPen(Qt::NoPen),
                                           QBrush(green_color));
    green_light->setZValue(86);
    traffic_light_items_.push_back(green_light);
  }
}

void PerspectiveView::updateFovVisualization(const core::Coordinate& position,
                                             double heading,
                                             double fov_radians,
                                             double range)
{
  if (fov_item_ == nullptr)
  {
    return;
  }

  const QPolygonF fov_polygon = createFovPolygon(heading, fov_radians, range);

  // For vertical road: swap x/y coordinates
  const double scene_x = position.y * scale_factor_;  // Lateral position
  const double scene_y = -position.x * scale_factor_; // Forward position

  QTransform transform;
  transform.translate(scene_x, scene_y);

  fov_item_->setPolygon(fov_polygon);
  fov_item_->setTransform(transform);
}

void PerspectiveView::clearScene()
{
  for (auto *item : lane_items_)
  {
    scene_->removeItem(item);
    delete item;
  }
  lane_items_.clear();

  for (auto *item : sign_items_)
  {
    scene_->removeItem(item);
    delete item;
  }
  sign_items_.clear();
}

void PerspectiveView::setScaleFactor(double scale)
{
  scale_factor_ = scale;
}

void PerspectiveView::setViewMode(ViewMode mode)
{
  view_mode_ = mode;
  // View will be applied on next updateEgoVehicle call
}

void PerspectiveView::cycleViewMode()
{
  switch (view_mode_)
  {
  case ViewMode::TopView:
    setViewMode(ViewMode::FrontView);
    break;
  case ViewMode::FrontView:
    setViewMode(ViewMode::BirdEye);
    break;
  case ViewMode::BirdEye:
    setViewMode(ViewMode::TopView);
    break;
  }
}

void PerspectiveView::resizeEvent(QResizeEvent *event)
{
  QGraphicsView::resizeEvent(event);
}

void PerspectiveView::wheelEvent(QWheelEvent *event)
{
  // Only adjust zoom level - the actual transform is applied in
  // updateEgoVehicle which runs every frame and keeps the car centered
  const double factor = (event->angleDelta().y() > 0) ? 1.15 : 0.85;
  zoom_level_ *= factor;

  // Clamp zoom level
  zoom_level_ = std::max(0.3, std::min(5.0, zoom_level_));

  // The view transform will be applied in the next updateEgoVehicle call
  // which centers on the car
}

QPolygonF PerspectiveView::createVehiclePolygon() const
{
  // Vehicle shape (arrow pointing UP for vertical road)
  const double length = core::EgoVehicle::kLength * scale_factor_;
  const double width = core::EgoVehicle::kWidth * scale_factor_;

  QPolygonF poly;
  poly << QPointF(0, -length / 2)         // Front tip (top)
       << QPointF(-width / 2, length / 2) // Back left (bottom-left)
       << QPointF(0, length / 3)          // Back center indent
       << QPointF(width / 2, length / 2); // Back right (bottom-right)

  return poly;
}

QPolygonF PerspectiveView::createFovPolygon(double heading,
                                            double fov_radians,
                                            double range) const
{
  const double half_fov = fov_radians / 2.0;
  const double scaled_range = range * scale_factor_;

  QPolygonF poly;
  poly << QPointF(0, 0); // Origin (vehicle position)

  // Create arc points - FoV cone pointing upward (negative Y in screen coords)
  constexpr int arc_segments = 20;
  for (int i = 0; i <= arc_segments; ++i)
  {
    // Angle calculation for upward-pointing FoV
    // Start from -90 degrees (pointing up) and apply heading + FoV spread
    const double base_angle = -M_PI / 2.0; // -90 degrees = pointing up
    const double angle = base_angle - heading - half_fov +
                         (static_cast<double>(i) / arc_segments) * fov_radians;
    const double x = scaled_range * std::cos(angle);
    const double y = scaled_range * std::sin(angle);
    poly << QPointF(x, y);
  }

  return poly;
}

QColor PerspectiveView::getSignColor(core::TrafficSignType type) const
{
  switch (type)
  {
  case core::TrafficSignType::Stop:
    return QColor(244, 67, 54); // Red
  case core::TrafficSignType::Yield:
    return QColor(255, 193, 7); // Yellow
  case core::TrafficSignType::SpeedLimit:
    return QColor(33, 150, 243); // Blue
  default:
    return QColor(158, 158, 158); // Grey
  }
}

void PerspectiveView::updateTurnSignal(core::TurnSignalState state)
{
  current_signal_state_ = state;

  // Update blink counter (blink every 15 frames at 60 FPS = ~4 Hz blink)
  blink_counter_++;
  if (blink_counter_ >= 15)
  {
    blink_counter_ = 0;
    signal_blink_on_ = !signal_blink_on_;
  }

  // Remove existing signal items
  if (left_signal_item_ != nullptr)
  {
    scene_->removeItem(left_signal_item_);
    delete left_signal_item_;
    left_signal_item_ = nullptr;
  }
  if (right_signal_item_ != nullptr)
  {
    scene_->removeItem(right_signal_item_);
    delete right_signal_item_;
    right_signal_item_ = nullptr;
  }

  // If signal is off, don't draw anything
  if (state == core::TurnSignalState::Off)
  {
    return;
  }

  // Only draw when blink is on
  if (!signal_blink_on_)
  {
    return;
  }

  const QColor signal_color(255, 165, 0); // Orange

  // Create arrow polygons relative to vehicle
  QPolygonF left_arrow;
  left_arrow << QPointF(-6, 0) << QPointF(-3, -2) << QPointF(-3, -1)
             << QPointF(-1, -1) << QPointF(-1, 1) << QPointF(-3, 1)
             << QPointF(-3, 2);

  QPolygonF right_arrow;
  right_arrow << QPointF(6, 0) << QPointF(3, -2) << QPointF(3, -1)
              << QPointF(1, -1) << QPointF(1, 1) << QPointF(3, 1)
              << QPointF(3, 2);

  if (state == core::TurnSignalState::Left ||
      state == core::TurnSignalState::Hazard)
  {
    left_signal_item_ =
        scene_->addPolygon(left_arrow, QPen(Qt::NoPen), QBrush(signal_color));
    left_signal_item_->setZValue(110);
    // Position relative to vehicle
    if (vehicle_item_ != nullptr)
    {
      left_signal_item_->setTransform(vehicle_item_->transform());
    }
  }

  if (state == core::TurnSignalState::Right ||
      state == core::TurnSignalState::Hazard)
  {
    right_signal_item_ =
        scene_->addPolygon(right_arrow, QPen(Qt::NoPen), QBrush(signal_color));
    right_signal_item_->setZValue(110);
    // Position relative to vehicle
    if (vehicle_item_ != nullptr)
    {
      right_signal_item_->setTransform(vehicle_item_->transform());
    }
  }
}

void PerspectiveView::updateNPCVehicles(
    const std::vector<core::NPCVehicle>& npcs)
{
  // Clear existing NPC items
  for (auto *item : npc_items_)
  {
    if (item != nullptr)
    {
      scene_->removeItem(item);
      delete item;
    }
  }
  npc_items_.clear();

  // Draw each NPC
  for (const auto& npc : npcs)
  {
    // Create NPC polygon pointing upward (vertical road orientation)
    const double half_length = core::NPCVehicle::kLength / 2.0;
    const double half_width = core::NPCVehicle::kWidth / 2.0;

    // Polygon pointing up (negative Y direction in screen coords)
    QPolygonF npc_poly;
    npc_poly << QPointF(0, -half_length)              // Front point (top)
             << QPointF(-half_width, half_length / 2) // Back left
             << QPointF(-half_width, half_length)     // Rear left
             << QPointF(half_width, half_length)      // Rear right
             << QPointF(half_width, half_length / 2); // Back right

    // Create graphics item with green color for NPCs
    auto *item =
        scene_->addPolygon(npc_poly,
                           QPen(QColor(20, 100, 40), 0.2), // Dark green border
                           QBrush(QColor(40, 180, 80, 200))); // Green fill

    // Position NPC - X is horizontal (lane), Y is vertical (progress along
    // road)
    const double scaled_x = npc.getY() * scale_factor_;  // Lane position
    const double scaled_y = -npc.getX() * scale_factor_; // Road progress

    QTransform transform;
    transform.translate(scaled_x, scaled_y);
    transform.scale(scale_factor_, scale_factor_);
    item->setTransform(transform);
    item->setZValue(80); // Below ego vehicle (100)

    npc_items_.push_back(item);
  }
}

void PerspectiveView::updatePedestrians(
    const std::vector<core::Pedestrian>& pedestrians)
{
  // Clear previous pedestrian items
  for (auto *item : pedestrian_items_)
  {
    scene_->removeItem(item);
    delete item;
  }
  pedestrian_items_.clear();

  if (pedestrians.empty())
  {
    return;
  }

  // Create pedestrian items (orange circles)
  for (const auto& pedestrian : pedestrians)
  {
    constexpr double kPedestrianRadius = 0.4;

    // Create ellipse for pedestrian
    auto *item =
        scene_->addEllipse(-kPedestrianRadius,
                           -kPedestrianRadius,
                           kPedestrianRadius * 2.0,
                           kPedestrianRadius * 2.0,
                           QPen(QColor(180, 80, 0), 0.15), // Dark orange border
                           QBrush(QColor(255, 140, 0, 200))); // Orange fill

    // Position pedestrian - X is horizontal (lane), Y is vertical (progress)
    const double scaled_x =
        pedestrian.getY() * scale_factor_; // Lateral position
    const double scaled_y = -pedestrian.getX() * scale_factor_; // Road progress

    QTransform transform;
    transform.translate(scaled_x, scaled_y);
    transform.scale(scale_factor_, scale_factor_);
    item->setTransform(transform);
    item->setZValue(90); // Above NPCs (80), below ego (100)

    pedestrian_items_.push_back(item);
  }
}

} // namespace ui
} // namespace adas
