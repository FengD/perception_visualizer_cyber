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
  auto global_data = Singleton<GlobalData>::get();

  QFont font;
  font.setPixelSize(18);

  auto layout = new QGridLayout();

  // hdmap
  auto label_hdmap = new QLabel("HDMap");
  label_hdmap->setAlignment(Qt::AlignCenter);
  label_hdmap->setFont(font);
  layout->addWidget(label_hdmap, 0, 0, Qt::AlignCenter);
  auto vbox_hdmap = new QVBoxLayout();
  vbox_hdmap->addLayout(makeItem("Road", global_data->config_.hdmap_road_color()));
  vbox_hdmap->addLayout(makeItem("Junction", global_data->config_.hdmap_junction_color()));
  vbox_hdmap->addLayout(makeItem("Side Area", global_data->config_.hdmap_side_area_color()));
  vbox_hdmap->addLayout(makeItem("Parking Area", global_data->config_.hdmap_parking_area_color()));
  vbox_hdmap->addLayout(makeItem("Walk Way", global_data->config_.hdmap_walk_way_color()));
  vbox_hdmap->addLayout(makeItem("Square", global_data->config_.hdmap_square_color()));
  vbox_hdmap->addLayout(makeItem("Green Belt", global_data->config_.hdmap_green_belt_color()));
  vbox_hdmap->addLayout(
      makeItem("Road Block Area", global_data->config_.hdmap_road_block_area_color()));
  vbox_hdmap->addLayout(
      makeItem("Lane Center Line", global_data->config_.hdmap_lane_center_line_color()));
  vbox_hdmap->addLayout(makeItem("Side Line", global_data->config_.hdmap_side_line_color()));
  vbox_hdmap->setSpacing(10);
  layout->addLayout(vbox_hdmap, 1, 0);

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

  // pnr
  auto label_pnc = new QLabel("PNC");
  label_pnc->setAlignment(Qt::AlignCenter);
  label_pnc->setFont(font);
  layout->addWidget(label_pnc, 0, 2, Qt::AlignCenter);
  auto vbox_pnc = new QVBoxLayout();
  vbox_pnc->addLayout(makeItem("Prediction", global_data->config_.prediction_path_color()));
  vbox_pnc->addLayout(makeItem("Trajectory", global_data->config_.planning_trajectory_color()));
  layout->addLayout(vbox_pnc, 1, 2);

  layout->setRowStretch(0, 1);
  layout->setRowStretch(1, 5);
  layout->setHorizontalSpacing(30);
  this->setLayout(layout);

  this->setWindowTitle("Info");
  const auto &screen_rect = QApplication::desktop()->screen()->rect();
  this->setMinimumSize(screen_rect.size() / 3);
  this->move(screen_rect.center() - this->rect().center());
}

}  // namespace airi
}  // namespace crdc
