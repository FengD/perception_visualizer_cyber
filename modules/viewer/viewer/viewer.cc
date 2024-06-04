#include <GL/glut.h>
#include <gflags/gflags.h>
#include <QApplication>
#include <QIcon>
#include <csignal>
#ifndef WITH_ROS2
#include "cyber/cyber.h"
#else
#include "common/macros.h"
#endif
#include "viewer/main_window.h"

QApplication *g_app = nullptr;

void onSignal(int sig) {
  if (sig == SIGINT) {
#ifndef WITH_ROS2
    apollo::cyber::Clear();
#endif
    if (g_app) {
      g_app->exit();
    }
  }
}

int main(int argc, char **argv) {
  google::ParseCommandLineFlags(&argc, &argv, true);
#ifndef WITH_ROS2
  apollo::cyber::Init(argv[0]);
#endif
  glutInit(&argc, argv);

  QApplication app(argc, argv);
  g_app = &app;
  std::signal(SIGINT, onSignal);
  app.setWindowIcon(QIcon(std::getenv("CRDC_WS") + QString("/icons/logo.png")));
  crdc::airi::MainWindow main_window;
  main_window.show();
  auto ret = app.exec();
#ifndef WITH_ROS2
  apollo::cyber::Clear();
#endif
  return ret;
}
