#pragma once

#include <QComboBox>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QWidget>
#include <mutex>
#include <opencv2/opencv.hpp>
#include <set>
#include <string>
#include <unordered_map>
#include <QCheckBox>
#include "viewer/message/message_hub.h"
#include "cyber/sensor_proto/image.pb.h"
#include "cyber/sensor_proto/image_marker.pb.h"

namespace crdc {
namespace airi {

class ImagePlayer : public QWidget {
  enum MousePosition { NORMAL = 0, RIGHT_BOTTOM, RIGHT, BOTTOM };

 public:
  ImagePlayer();

 protected:
  void paintEvent(QPaintEvent *event) override;

 protected:
  void mousePressEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;

 protected:
  QComboBox *cb_channel_;
  QCheckBox *cb_marker_;
  std::set<std::string> seen_channels_;
  std::set<std::string> show_channels_;
  std::unordered_map<std::string, std::shared_ptr<crdc::airi::Image2>> proto_images_;
  std::unordered_map<std::string, cv::Mat> images_;
  std::unordered_map<std::string, bool> needs_update_;
  std::mutex mutex_;

  std::unordered_map<std::string, std::shared_ptr<ImageMarkerList>> marker_lists_;
  std::mutex mutex_marker_;

  MousePosition mouse_pos_;
  bool is_dragging_;
  QPoint pos_start_;
  QSize sz_start_;
};

}  // namespace airi
}  // namespace crdc
