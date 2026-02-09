/**
 * @file speed_limit_widget.cpp
 * @brief Speed limit detection widget implementation
 */

#include "speed_limit_widget.hpp"
#include <QFont>
#include <QPainter>
#include <QPainterPath>

namespace adas
{
namespace ui
{

SpeedLimitWidget::SpeedLimitWidget(QWidget *parent)
    : QWidget(parent), layout_(std::make_unique<QVBoxLayout>(this)),
      status_label_(std::make_unique<QLabel>())
{
  setupUi();
}

void SpeedLimitWidget::setupUi()
{
  // Use fixed size hint but allow layout flexibility
  setFixedSize(120, 130); // Fixed size to avoid layout recalculation

  // Status label (below the sign)
  QFont status_font;
  status_font.setPointSize(10);
  status_font.setBold(true);
  status_label_->setFont(status_font);
  status_label_->setAlignment(Qt::AlignCenter);
  status_label_->setStyleSheet("color: #757575;");

  layout_->setContentsMargins(5, 80, 5, 5); // Top margin for the painted sign
  layout_->setSpacing(4);
  layout_->addStretch();
  layout_->addWidget(status_label_.get());

  updateDisplay();
}

void SpeedLimitWidget::updateLimit(std::optional<std::uint32_t> speedLimit,
                                   double /*distance*/)
{
  if (speedLimit.has_value())
  {
    // Only emit signal if the limit actually changed
    if (active_limit_ != speedLimit)
    {
      active_limit_ = speedLimit;
      emit speedLimitChanged(active_limit_);
    }
  }
  // If no new limit detected, keep the existing one (persistent display)

  updateDisplay();
  update(); // Trigger repaint
}

void SpeedLimitWidget::clearLimit()
{
  if (active_limit_.has_value())
  {
    active_limit_ = std::nullopt;
    is_exceeding_ = false;
    emit speedLimitChanged(std::nullopt);
    updateDisplay();
    update();
  }
}

std::optional<std::uint32_t> SpeedLimitWidget::getCurrentLimit() const noexcept
{
  return active_limit_;
}

void SpeedLimitWidget::setExceedingWarning(bool exceeding)
{
  if (is_exceeding_ != exceeding)
  {
    is_exceeding_ = exceeding;
    update(); // Trigger repaint to show warning state
  }
}

bool SpeedLimitWidget::isExceeding() const noexcept
{
  return is_exceeding_;
}

void SpeedLimitWidget::updateDisplay()
{
  if (active_limit_.has_value())
  {
    status_label_->setText("✓ ACTIVE");
    status_label_->setStyleSheet("color: #4caf50; font-weight: bold;");
  }
  else
  {
    status_label_->setText("NO LIMIT");
    status_label_->setStyleSheet("color: #757575;");
  }
}

void SpeedLimitWidget::paintEvent(QPaintEvent * /*event*/)
{
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);

  const int centerX = width() / 2;
  const int signRadius = 45;
  const int borderWidth = 6;
  const int innerRadius = signRadius - borderWidth;

  // Draw the circular speed limit sign
  // Outer red border
  QColor borderColor = is_exceeding_ ? QColor("#ffeb3b") : QColor("#d32f2f");
  painter.setPen(Qt::NoPen);
  painter.setBrush(borderColor);
  painter.drawEllipse(QPoint(centerX, signRadius + 5), signRadius, signRadius);

  // Inner white circle
  painter.setBrush(Qt::white);
  painter.drawEllipse(
      QPoint(centerX, signRadius + 5), innerRadius, innerRadius);

  // Speed limit text
  if (active_limit_.has_value())
  {
    QFont speedFont;
    speedFont.setPointSize(24);
    speedFont.setBold(true);
    painter.setFont(speedFont);

    // Text color changes if exceeding
    painter.setPen(is_exceeding_ ? QColor("#d32f2f") : Qt::black);

    QString speedText = QString::number(active_limit_.value());
    painter.drawText(
        QRect(centerX - innerRadius, 5, innerRadius * 2, signRadius * 2),
        Qt::AlignCenter,
        speedText);
  }
  else
  {
    // No limit - show dashes
    QFont dashFont;
    dashFont.setPointSize(20);
    dashFont.setBold(true);
    painter.setFont(dashFont);
    painter.setPen(QColor("#999999"));
    painter.drawText(
        QRect(centerX - innerRadius, 5, innerRadius * 2, signRadius * 2),
        Qt::AlignCenter,
        "--");
  }

  // "km/h" label below the number
  if (active_limit_.has_value())
  {
    QFont unitFont;
    unitFont.setPointSize(8);
    painter.setFont(unitFont);
    painter.setPen(Qt::darkGray);
    painter.drawText(
        QRect(centerX - innerRadius, signRadius - 5, innerRadius * 2, 30),
        Qt::AlignCenter,
        "km/h");
  }
}

} // namespace ui
} // namespace adas
