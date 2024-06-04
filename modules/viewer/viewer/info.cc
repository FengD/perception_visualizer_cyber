#include "viewer/info.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QLabel>
#include <QLayout>
#include <sstream>
#include "viewer/global_data.h"

namespace crdc {
namespace airi {

QString generateStyleSheet(const crdc::airi::viewer::Color &color) {
  std::stringstream ss;
  ss << "background-color: rgba(";
  ss << color.r() * 255 << ", ";
  ss << color.g() * 255 << ", ";
  ss << color.b() * 255 << ", ";
  ss << color.a() * 255 << ")";
  return QString::fromStdString(ss.str());
}

QHBoxLayout *makeItem(const QString &name, const crdc::airi::viewer::Color &color) {
  auto hbox = new QHBoxLayout();
  auto label = new QLabel(name);
  auto label_rect = new QLabel();
  QFont font;
  font.setPixelSize(14);
  label->setFont(font);
  label_rect->setStyleSheet(generateStyleSheet(color));
  hbox->addWidget(label);
  hbox->addWidget(label_rect);
  return hbox;
}

Info::Info() {
  auto global_data = crdc::airi::common::Singleton<GlobalData>::get();

  QFont font;
  font.setPixelSize(18);

  auto layout = new QGridLayout();

  // perception
  auto label_perception = new QLabel("Perception");
  label_perception->setAlignment(Qt::AlignCenter);
  label_perception->setFont(font);
  layout->addWidget(label_perception, 0, 1, Qt::AlignCenter);
  auto vbox_perception = new QVBoxLayout();
  vbox_perception->addLayout(makeItem("Vehicle", global_data->config_.perception_color_vehicle()));
  vbox_perception->addLayout(makeItem("Cyclist", global_data->config_.perception_color_cyclist()));
  vbox_perception->addLayout(
      makeItem("Pedestrian", global_data->config_.perception_color_pedestrian()));
  vbox_perception->addLayout(
      makeItem("Unknown Movable", global_data->config_.perception_color_unknown_movable()));
  vbox_perception->addLayout(
      makeItem("Unknown Unmovable", global_data->config_.perception_color_unknown_unmovable()));
  vbox_perception->addLayout(
      makeItem("Traffic Cone", global_data->config_.perception_color_traffic_cone()));
  vbox_perception->addLayout(makeItem("Fence", global_data->config_.perception_color_fence()));
  vbox_perception->setSpacing(10);
  layout->addLayout(vbox_perception, 1, 1);

  layout->setRowStretch(0, 1);
  layout->setRowStretch(1, 5);
  layout->setHorizontalSpacing(30);
  this->setLayout(layout);

  this->setWindowTitle("Tips");
  const auto &screen_rect = QApplication::desktop()->screen()->rect();
  this->setMinimumSize(screen_rect.size() / 3);
  this->move(screen_rect.center() - this->rect().center());
}

}  // namespace airi
}  // namespace crdc
