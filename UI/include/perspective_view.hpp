//! @file perspective_view.hpp
//! @brief 2D top-down graphical view using QGraphicsView
//!

#ifndef ADAS_UI_PERSPECTIVE_VIEW_HPP
#define ADAS_UI_PERSPECTIVE_VIEW_HPP

#include "ego_vehicle.hpp"
#include "road_segment.hpp"
#include "traffic_light.hpp"
#include "traffic_sign.hpp"
#include <QGraphicsEllipseItem>
#include <QGraphicsItem>
#include <QGraphicsPolygonItem>
#include <QGraphicsRectItem>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QPolygonF>
#include <memory>
#include <vector>

namespace adas
{
namespace ui
{

//! @brief View mode for the perspective display
//!
enum class ViewMode
{
  TopView,   ///< Top-down view centered on vehicle
  FrontView, ///< View from behind the vehicle looking forward
  BirdEye    ///< High altitude bird's eye view of entire road
};
//! @brief 2D top-down perspective view of the ADAS scene
//!
class PerspectiveView : public QGraphicsView
{
  Q_OBJECT

public:
  explicit PerspectiveView(QWidget *parent = nullptr);
  ~PerspectiveView() override = default;

  //! @brief Update the ego vehicle display
  //! @param vehicle Current ego vehicle state
  //!
  void updateEgoVehicle(const core::EgoVehicle& vehicle);

  //! @brief Update road segment display
  //! @param road Road segment to display
  //!
  void updateRoad(const core::RoadSegment& road);

  //! @brief Update traffic signs display
  //! @param signs Traffic signs to display
  //! @param detected_ids IDs of signs currently detected
  //!
  void updateSigns(const std::vector<core::TrafficSign>& signs,
                   const std::vector<core::SignId>& detected_ids = {});

  //! @brief Update traffic lights display
  //! @param lights Traffic lights to display
  //! @param vehicle_lane_id Lane ID of the vehicle (stop lines only drawn for this lane)
  //!
  void updateTrafficLights(const std::vector<core::TrafficLight>& lights,
                           core::LaneId vehicle_lane_id = 0U);

  //! @brief Update FoV visualization
  //! @param position Vehicle position
  //! @param heading Vehicle heading in radians
  //! @param fov_radians Field of view angle
  //! @param range Maximum FoV range
  //!
  void updateFovVisualization(const core::Coordinate& position,
                              double heading,
                              double fov_radians,
                              double range);

  //! @brief Clear all items from the scene
  //!
  void clearScene();

  //! @brief Set the view scale factor
  //!
  void setScaleFactor(double scale);

  //! @brief Set the view mode
  //! @param mode New view mode
  //!
  void setViewMode(ViewMode mode);

  //! @brief Get current view mode
  //!
  [[nodiscard]] ViewMode getViewMode() const noexcept
  {
    return view_mode_;
  }

  //! @brief Cycle to next view mode
  //!
  void cycleViewMode();

protected:
  void resizeEvent(QResizeEvent *event) override;
  void wheelEvent(QWheelEvent *event) override;

private:
  std::unique_ptr<QGraphicsScene> scene_;

  // Scene items (managed by scene, stored for updates)
  QGraphicsPolygonItem *vehicle_item_{nullptr};
  QGraphicsPolygonItem *fov_item_{nullptr};
  std::vector<QGraphicsRectItem *> lane_items_;
  std::vector<QGraphicsItem *> sign_items_;
  std::vector<QGraphicsItem *> traffic_light_items_;

  double scale_factor_{5.0}; // Pixels per meter
  double zoom_level_{1.0};   // User-adjustable zoom multiplier
  ViewMode view_mode_{ViewMode::TopView};
  double last_vehicle_x_{0.0};
  double last_vehicle_y_{0.0};
  double last_vehicle_heading_{0.0};

  void setupScene();
  [[nodiscard]] QPolygonF createVehiclePolygon() const;
  [[nodiscard]] QPolygonF
  createFovPolygon(double heading, double fov_radians, double range) const;
  [[nodiscard]] QColor getSignColor(core::TrafficSignType type) const;
};

} // namespace ui
} // namespace adas

#endif // ADAS_UI_PERSPECTIVE_VIEW_HPP
