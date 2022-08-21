#include "viewer/main_window.h"
#include <gflags/gflags.h>
#include <QApplication>
#include <QDesktopWidget>
#include <QLayout>
#include <QSplitter>
#include "common/io/file.h"
#include "viewer/global_data.h"
#include "viewer/glwidget.h"
#include "viewer/image_player.h"
#include "viewer/renderer_manager.h"
#include "viewer/toolbar.h"

namespace crdc {
namespace airi {

MainWindow::MainWindow() {
  // initialize global data
  auto global_data = Singleton<GlobalData>::get();
  global_data->initialize();

  // init widgets
  global_data->main_window_ = this;
  global_data->glwidget_ = new GLWidget();
  global_data->image_player_ = new ImagePlayer();
  // global_data->chart_widget_ = new ChartWidget();
  global_data->renderer_manager_ = new RendererManager();
  global_data->toolbar_ = new Toolbar();

  // init layout
  auto left_container = new QWidget();
  auto left_vbox = new QVBoxLayout();
  left_vbox->setMargin(0);
  left_vbox->setSpacing(0);
  left_vbox->addWidget(global_data->toolbar_);
  left_vbox->addWidget(global_data->glwidget_);
  left_container->setLayout(left_vbox);
  auto splitter = new QSplitter(Qt::Horizontal);
  splitter->setStyleSheet("QSplitter::handle { border: 1px dashed gray }");
  splitter->setChildrenCollapsible(false);
  splitter->addWidget(left_container);
  // splitter->addWidget(global_data->chart_widget_);
  splitter->addWidget(global_data->renderer_manager_);
  auto layout = new QHBoxLayout();
  layout->addWidget(splitter);
  layout->setMargin(0);
  layout->setSpacing(0);
  this->setLayout(layout);

  // set minimum size and move to center of screen
  const auto &screen_rect = QApplication::desktop()->screen()->rect();
  this->setMinimumSize(screen_rect.size() * 3 / 4);
  this->adjustSize();
  this->move(screen_rect.center() - rect().center());
}

void MainWindow::keyPressEvent(QKeyEvent *e) {
  switch (e->key()) {
    case Qt::Key_F11:
      if (this->isFullScreen()) {
        this->showNormal();
      } else {
        this->showFullScreen();
      }
      break;

    default:;
  }
}

}  // namespace airi
}  // namespace crdc
