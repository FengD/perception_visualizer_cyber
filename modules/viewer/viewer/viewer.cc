#include <GL/glut.h>
#include <gflags/gflags.h>
#include <QApplication>
#include <QIcon>
#include <csignal>
#include "cyber/cyber.h"
#include "viewer/main_window.h"

QApplication *g_app = nullptr;

void onSignal(int sig) {
  if (sig == SIGINT) {
    apollo::cyber::Clear();
    if (g_app) {
      g_app->exit();
    }
  }
}

int main(int argc, char **argv) {
  apollo::cyber::Init(argv[0]);
  glutInit(&argc, argv);

  QApplication app(argc, argv);
  g_app = &app;
  std::signal(SIGINT, onSignal);
  app.setWindowIcon(QIcon(std::getenv("CRDC_WS") + QString("/icons/dream.png")));
  crdc::airi::MainWindow main_window;
  main_window.show();
  auto ret = app.exec();
  apollo::cyber::Clear();
  return ret;
}
