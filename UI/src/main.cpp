/**
 * @file main.cpp
 * @brief ADAS Simulation entry point
 */

#include "main_window.hpp"
#include <QApplication>

/**
 * @brief Application entry point
 * @param argc Argument count
 * @param argv Argument vector
 * @return Exit code
 */
int main(int argc, char *argv[])
{
  // Note: Qt6 has high-DPI scaling enabled by default, no need for
  // AA_EnableHighDpiScaling

  QApplication app(argc, argv);
  app.setApplicationName("ADAS Simulation");
  app.setApplicationVersion("1.0.0");
  app.setOrganizationName("ADAS Project");

  // Apply dark theme palette
  QPalette dark_palette;
  dark_palette.setColor(QPalette::Window, QColor(10, 10, 20));
  dark_palette.setColor(QPalette::WindowText, Qt::white);
  dark_palette.setColor(QPalette::Base, QColor(15, 15, 30));
  dark_palette.setColor(QPalette::AlternateBase, QColor(20, 20, 40));
  dark_palette.setColor(QPalette::ToolTipBase, Qt::white);
  dark_palette.setColor(QPalette::ToolTipText, Qt::white);
  dark_palette.setColor(QPalette::Text, Qt::white);
  dark_palette.setColor(QPalette::Button, QColor(30, 30, 60));
  dark_palette.setColor(QPalette::ButtonText, Qt::white);
  dark_palette.setColor(QPalette::BrightText, Qt::red);
  dark_palette.setColor(QPalette::Link, QColor(66, 165, 245));
  dark_palette.setColor(QPalette::Highlight, QColor(66, 165, 245));
  dark_palette.setColor(QPalette::HighlightedText, Qt::black);
  app.setPalette(dark_palette);

  adas::ui::MainWindow main_window;
  main_window.show();
  main_window.startSimulation();

  return app.exec();
}
