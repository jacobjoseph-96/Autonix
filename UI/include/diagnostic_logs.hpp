//! @file diagnostic_logs.hpp
//! @brief Diagnostic logging widget with timestamped entries
//!

#ifndef ADAS_UI_DIAGNOSTIC_LOGS_HPP
#define ADAS_UI_DIAGNOSTIC_LOGS_HPP

#include <QDateTime>
#include <QString>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QWidget>
#include <memory>

namespace adas
{
namespace ui
{

//! @brief Log severity levels
//!
enum class LogLevel
{
  Info,
  Warning,
  Error,
  Safety
};

//! @brief Diagnostic log widget for system state and safety violations
//!
class DiagnosticLogs : public QWidget
{
  Q_OBJECT

public:
  explicit DiagnosticLogs(QWidget *parent = nullptr);
  ~DiagnosticLogs() override = default;

  //! @brief Log an informational message
  //! @param message Log message
  //!
  void logInfo(const QString& message);

  //! @brief Log a warning message
  //! @param message Log message
  //!
  void logWarning(const QString& message);

  //! @brief Log an error message
  //! @param message Log message
  //!
  void logError(const QString& message);

  //! @brief Log a safety violation
  //! @param message Safety violation description
  //!
  void logSafetyViolation(const QString& message);

  //! @brief Log with custom level
  //! @param level Log severity level
  //! @param message Log message
  //!
  void log(LogLevel level, const QString& message);

  //! @brief Clear all log entries
  //!
  void clear();

  //! @brief Get all log content as plain text
  //!
  [[nodiscard]] QString getLogContent() const;

public slots:
  //! @brief Slot for state change notifications
  //! @param component Component name
  //! @param old_state Previous state
  //! @param new_state New state
  //!
  void onStateChanged(const QString& component,
                      const QString& old_state,
                      const QString& new_state);

private:
  std::unique_ptr<QVBoxLayout> layout_;
  std::unique_ptr<QTextEdit> text_edit_;

  //! @brief Format log entry with timestamp and level
  //!
  [[nodiscard]] QString formatLogEntry(LogLevel level,
                                       const QString& message) const;

  //! @brief Get color for log level
  //!
  [[nodiscard]] static QString getLevelColor(LogLevel level);

  //! @brief Get prefix for log level
  //!
  [[nodiscard]] static QString getLevelPrefix(LogLevel level);
};

} // namespace ui
} // namespace adas

#endif // ADAS_UI_DIAGNOSTIC_LOGS_HPP
