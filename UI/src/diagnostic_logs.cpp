/**
 * @file diagnostic_logs.cpp
 * @brief Diagnostic logging widget implementation
 */

#include "diagnostic_logs.hpp"

namespace adas
{
namespace ui
{

DiagnosticLogs::DiagnosticLogs(QWidget *parent)
    : QWidget(parent), layout_(std::make_unique<QVBoxLayout>(this)),
      text_edit_(std::make_unique<QTextEdit>())
{
  text_edit_->setReadOnly(true);
  text_edit_->setStyleSheet("QTextEdit {"
                            "  background-color: #1a1a2e;"
                            "  color: #eaeaea;"
                            "  font-family: 'Consolas', 'Monaco', monospace;"
                            "  font-size: 11px;"
                            "  border: none;"
                            "  padding: 8px;"
                            "}");

  layout_->setContentsMargins(0, 0, 0, 0);
  layout_->addWidget(text_edit_.get());

  // Log initialization
  logInfo("ADAS Diagnostic System initialized");
}

void DiagnosticLogs::logInfo(const QString& message)
{
  log(LogLevel::Info, message);
}

void DiagnosticLogs::logWarning(const QString& message)
{
  log(LogLevel::Warning, message);
}

void DiagnosticLogs::logError(const QString& message)
{
  log(LogLevel::Error, message);
}

void DiagnosticLogs::logSafetyViolation(const QString& message)
{
  log(LogLevel::Safety, message);
}

void DiagnosticLogs::log(LogLevel level, const QString& message)
{
  const QString formatted = formatLogEntry(level, message);
  text_edit_->append(formatted);

  // Auto-scroll to bottom
  auto cursor = text_edit_->textCursor();
  cursor.movePosition(QTextCursor::End);
  text_edit_->setTextCursor(cursor);
}

void DiagnosticLogs::clear()
{
  text_edit_->clear();
}

QString DiagnosticLogs::getLogContent() const
{
  return text_edit_->toPlainText();
}

void DiagnosticLogs::onStateChanged(const QString& component,
                                    const QString& old_state,
                                    const QString& new_state)
{
  const QString message =
      QString("%1: %2 → %3").arg(component).arg(old_state).arg(new_state);
  logInfo(message);
}

QString DiagnosticLogs::formatLogEntry(LogLevel level,
                                       const QString& message) const
{
  const QString timestamp =
      QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
  const QString color = getLevelColor(level);
  const QString prefix = getLevelPrefix(level);

  return QString("<span style='color: #666;'>[%1]</span> "
                 "<span style='color: %2; font-weight: bold;'>%3</span> "
                 "<span style='color: #eaeaea;'>%4</span>")
      .arg(timestamp)
      .arg(color)
      .arg(prefix)
      .arg(message.toHtmlEscaped());
}

QString DiagnosticLogs::getLevelColor(LogLevel level)
{
  switch (level)
  {
  case LogLevel::Info:
    return "#4fc3f7"; // Light blue
  case LogLevel::Warning:
    return "#ffb74d"; // Orange
  case LogLevel::Error:
    return "#ef5350"; // Red
  case LogLevel::Safety:
    return "#ff1744"; // Bright red
  default:
    return "#eaeaea";
  }
}

QString DiagnosticLogs::getLevelPrefix(LogLevel level)
{
  switch (level)
  {
  case LogLevel::Info:
    return "INFO";
  case LogLevel::Warning:
    return "WARN";
  case LogLevel::Error:
    return "ERROR";
  case LogLevel::Safety:
    return "⚠ SAFETY";
  default:
    return "LOG";
  }
}

} // namespace ui
} // namespace adas
