#include "viewer/renderer_manager.h"
#include <QFileDialog>
#include <QIcon>
#include <QLabel>
#include <QLayout>
#include <QScrollArea>
#include <QScrollBar>
#include <QSizePolicy>
#include "common/io/file.h"
#include "viewer/global_data.h"
#include "viewer/glwidget.h"
#include "viewer/widgets/push_button.h"
#include "viewer/widgets/renderer_item.h"

namespace crdc {
namespace airi {

RendererManager::RendererManager() {
  // button: import config
  auto bt_import = new PushButton(QIcon(std::getenv("CRDC_WS") + QString("/icons/import.png")), [&]() {
    auto path = QFileDialog::getOpenFileName(nullptr, "Import Configuration", "",
                                             tr("Proto text(*.prototxt);;All Files(*.*)"));
    if (!path.isEmpty()) {
     viewer::Config cfg;
      if (crdc::airi::util::get_proto_from_file(path.toStdString(), &cfg)) {
        crdc::airi::common::Singleton<GlobalData>::get()->config_ = cfg;
        crdc::airi::common::Singleton<GlobalData>::get()->glwidget_->loadConfigPost();
      }
    }
  });
  bt_import->setText("Import");
  bt_import->setFixedHeight(36);
  bt_import->setIconSize(QSize(28, 28));

  // button: export config
  auto bt_export = new PushButton(QIcon(std::getenv("CRDC_WS") + QString("/icons/export.png")), [&]() {
    auto path = QFileDialog::getSaveFileName(nullptr, "Export Configuration", "", "*.*");
    if (!path.isEmpty()) {
      crdc::airi::util::set_proto_to_ascii_file(crdc::airi::common::Singleton<GlobalData>::get()->config_,
                                               path.toStdString());
    }
  });
  bt_export->setText("Export");
  bt_export->setFixedHeight(36);
  bt_export->setIconSize(QSize(28, 28));

  this->setFocusPolicy(Qt::NoFocus);
  hbox_ = new QHBoxLayout();
  vbox_ = new QVBoxLayout();
  layout_ = new QVBoxLayout();

  hbox_->setMargin(0);
  hbox_->addWidget(bt_import);
  hbox_->addWidget(bt_export);
  layout_->addLayout(hbox_);

  container_ = new QWidget();
  scroll_area_ = new QScrollArea();
  container_->setLayout(vbox_);
  container_->setFixedHeight(2048);
  scroll_area_->setWidget(container_);
  scroll_area_->horizontalScrollBar()->hide();
  layout_->addWidget(scroll_area_);
  vbox_->setSpacing(0);
  vbox_->setMargin(0);
  vbox_->setAlignment(Qt::AlignTop);
  layout_->setContentsMargins(5, 3, 5, 5);
  layout_->setSpacing(5);
  layout_->setAlignment(Qt::AlignTop);
  this->setLayout(layout_);
}

void RendererManager::addWidget(QWidget *widget) { vbox_->addWidget(widget); }

void RendererManager::resizeEvent(QResizeEvent *e) {
  scroll_area_->setMaximumSize(e->size());
  container_->setFixedWidth(e->size().width() - 25);
  QWidget::resizeEvent(e);
}

}  // namespace airi
}  // namespace crdc
