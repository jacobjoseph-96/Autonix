//! @file speed_limit_widget.hpp
//! @brief Speed limit detection HUD widget with circular sign display
//!

#ifndef ADAS_UI_SPEED_LIMIT_WIDGET_HPP
#define ADAS_UI_SPEED_LIMIT_WIDGET_HPP

#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>
#include <cstdint>
#include <memory>
#include <optional>

namespace adas
{
namespace ui
{

//! @brief HUD widget displaying the currently active speed limit
//! @details Renders a circular sign-style display (red border, white center)
//!          with the speed value prominently shown. Persists the detected limit
//!          until a new speed limit is detected.
//!
class SpeedLimitWidget : public QWidget
{
  Q_OBJECT

public:
  explicit SpeedLimitWidget(QWidget *parent = nullptr);
  ~SpeedLimitWidget() override = default;

  //! @brief Update the displayed speed limit
  //! @param speedLimit Speed limit value in km/h (nullopt to clear)
  //! @param distance Distance to the sign in meters
  //!
  void updateLimit(std::optional<std::uint32_t> speedLimit, double distance);

  //! @brief Clear the speed limit (e.g., when changing roads)
  //!
  void clearLimit();

  //! @brief Get the current active speed limit
  //! @return Current limit in km/h, or nullopt if none active
  //!
  [[nodiscard]] std::optional<std::uint32_t> getCurrentLimit() const noexcept;

  //! @brief Set warning state when vehicle exceeds limit
  //! @param exceeding True if current speed exceeds limit
  //!
  void setExceedingWarning(bool exceeding);

  //! @brief Check if currently in warning state
  //!
  [[nodiscard]] bool isExceeding() const noexcept;

signals:
  //! @brief Emitted when the active speed limit changes
  //!
  void speedLimitChanged(std::optional<std::uint32_t> newLimit);

protected:
  void paintEvent(QPaintEvent *event) override;

private:
  std::optional<std::uint32_t> active_limit_{std::nullopt};
  bool is_exceeding_{false};

  std::unique_ptr<QVBoxLayout> layout_;
  std::unique_ptr<QLabel> status_label_;

  void setupUi();
  void updateDisplay();
};

} // namespace ui
} // namespace adas

#endif // ADAS_UI_SPEED_LIMIT_WIDGET_HPP
