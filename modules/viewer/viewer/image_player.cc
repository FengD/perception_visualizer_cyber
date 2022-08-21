#include "viewer/image_player.h"
#include <QCheckBox>
#include <QPainter>
#include <QTimer>
#include "viewer/util/image_conversion.h"
#include "viewer/global_data.h"
#include "viewer/glwidget.h"

namespace crdc {
namespace airi {

#ifdef CLAMP
#undef CLAMP
#endif
#define CLAMP(x, min, max) (x < min ? min : (x > max ? max : x))

void applyImageMarkerList(cv::Mat &image, const ImageMarkerList &marker_list) {
  for (const auto &marker : marker_list.markers()) {
    // setup color
    cv::Scalar color(255, 255, 255);
    if (marker.color().has_color3b()) {
      color = cv::Scalar(marker.color().color3b().r(), marker.color().color3b().g(),
                         marker.color().color3b().b());
    } else if (marker.color().has_color3f()) {
      color = cv::Scalar(marker.color().color3f().r() * 255, marker.color().color3f().g() * 255,
                         marker.color().color3f().b() * 255);
    } else if (marker.color().has_color4f()) {
      color = cv::Scalar(marker.color().color4f().r() * 255, marker.color().color4f().g() * 255,
                         marker.color().color4f().b() * 255, marker.color().color4f().a() * 255);
    }

    // draw marker to image
    if (marker.has_points()) {
      for (const auto &pt : marker.points().points()) {
        cv::circle(image, cv::Point(pt.x(), pt.y()), marker.size(), color, -1);
      }
    } else if (marker.has_lines()) {
      const int num_pairs = marker.lines().points_size() / 2;
      for (int i = 0; i < num_pairs; ++i) {
        const auto &from = marker.lines().points(i * 2);
        const auto &to = marker.lines().points(i * 2 + 1);
        cv::line(image, cv::Point(from.x(), from.y()), cv::Point(to.x(), to.y()), color,
                 marker.size());
      }
    } else if (marker.has_ellipse()) {
      cv::ellipse(image, cv::Point(marker.ellipse().center().x(), marker.ellipse().center().y()),
                  cv::Size(marker.ellipse().a(), marker.ellipse().b()), marker.ellipse().angle(), 0,
                  M_PI * 2, color, marker.size());
    } else if (marker.has_circle()) {
      cv::circle(image, cv::Point(marker.circle().center().x(), marker.circle().center().y()),
                 marker.circle().radius(), color, marker.size());
    } else if (marker.has_rect()) {
      cv::rectangle(image, cv::Point(marker.rect().left_top().x(), marker.rect().left_top().y()),
                    cv::Point(marker.rect().right_bottom().x(), marker.rect().right_bottom().y()),
                    color, marker.size());
    } else if (marker.has_text()) {
      cv::putText(
          image, marker.text().text(), cv::Point(marker.text().pos().x(), marker.text().pos().y()),
          cv::FONT_HERSHEY_SIMPLEX | cv::FONT_ITALIC, marker.text().scale(), color, marker.size());
    } else if (marker.has_cube()) {
      if (marker.cube().points_size() == 8) {
        const static std::vector<std::pair<int, int>> pairs{{0, 1}, {1, 2}, {2, 3}, {3, 0},
                                                            {4, 5}, {5, 6}, {6, 7}, {7, 4},
                                                            {0, 4}, {1, 5}, {2, 6}, {3, 7}};
        for (const auto &pair : pairs) {
          const auto &pt1 = marker.cube().points(pair.first);
          const auto &pt2 = marker.cube().points(pair.second);
          cv::line(image, cv::Point(pt1.x(), pt1.y()), cv::Point(pt2.x(), pt2.y()), color,
                   marker.size());
        }

        for (const auto &pt : marker.cube().points()) {
          cv::circle(image, cv::Point(pt.x(), pt.y()), 5, cv::Scalar(255, 255, 255));
        }
      }
    }
  }
}

ImagePlayer::ImagePlayer() {
  auto global_data = Singleton<GlobalData>::get();

  this->setFocusPolicy(Qt::NoFocus);
  this->setParent(global_data->glwidget_);
  this->setGeometry(0, 0, 200, 100);
  this->setStyleSheet("background-color: transparent");
  this->setMouseTracking(true);
  this->setHidden(true);

  cb_channel_ = new QComboBox(this);
  cb_channel_->setGeometry(0, 0, this->width() / 2, 20);
  cb_channel_->addItem("OFF");
  cb_channel_->setStyleSheet("background-color: gray");
  cb_channel_->setFocusPolicy(Qt::NoFocus);

  cb_marker_ = new QCheckBox("Marker", this);
  cb_marker_->setStyleSheet("background-color: gray");
  cb_marker_->setGeometry(this->width() / 2, 0, std::min(this->width() / 2, 70), 20);

  global_data->message_hub_->subscribe<crdc::airi::Image2>(
      [&](const std::string &channel, const std::shared_ptr<crdc::airi::Image2> &msg) {
        std::lock_guard<std::mutex> lock(mutex_);
        proto_images_[channel] = msg;
        needs_update_[channel] = true;
        seen_channels_.insert(channel);
      });

  global_data->message_hub_->subscribe<ImageMarkerList>(
      [&](const std::string &channel, const std::shared_ptr<ImageMarkerList> &msg) {
        std::lock_guard<std::mutex> lock(mutex_marker_);
        marker_lists_[channel] = msg;
      });

  auto timer = new QTimer();
  QObject::connect(timer, &QTimer::timeout, [&]() { this->update(); });
  timer->start(100);
}

void ImagePlayer::paintEvent(QPaintEvent *) {
  std::lock_guard<std::mutex> lock(mutex_);
  for (const auto &channel : seen_channels_) {
    if (show_channels_.find(channel) == show_channels_.end()) {
      show_channels_.insert(channel);
      cb_channel_->addItem(QString::fromStdString(channel));
    }
  }

  const auto &channel = cb_channel_->currentText().toStdString();
  if (channel == "OFF") {
    return;
  }

  if (needs_update_[channel]) {
    needs_update_[channel] = false;
    crdc::airi::util::convert_from_proto(*proto_images_[channel], &images_[channel]);
    if (proto_images_[channel]->type() == std::to_string(crdc::airi::BGR8)) {
      cv::cvtColor(images_[channel], images_[channel], CV_BGR2RGB);
    }
  }

  cv::Mat image = images_[channel].clone();

  // if (cb_marker_->isChecked()) {
  //   // apply self-contained marker lists
  //   for (const auto &marker_list : proto_images_[channel]->marker_lists()) {
  //     applyImageMarkerList(image, marker_list);
  //   }

  //   // apply image marker lists
  //   std::lock_guard<std::mutex> lock_marker(mutex_marker_);
  //   for (const auto &marker_list : marker_lists_) {
  //     if (marker_list.second->image_channel() == channel) {
  //       applyImageMarkerList(image, *(marker_list.second));
  //     }
  //   }
  // }

  auto qimg = QImage(image.data, image.cols, image.rows, QImage::Format::Format_RGB888);

  // adjust widget to fit image ratio
  float image_ratio = float(qimg.height()) / qimg.width();
  float widget_ratio = float(height()) / width();
  if (std::fabs(image_ratio - widget_ratio) > 0.01f) {
    setFixedHeight(width() * image_ratio);
  }

  // rescale image to fit widget size
  qimg = qimg.scaled(this->width(), this->height());

  // paint image
  auto painter = new QPainter(this);
  painter->drawImage(QPoint(0, 0), qimg);
  painter->end();
}

void ImagePlayer::mousePressEvent(QMouseEvent *event) {
  if (event->button() == Qt::MouseButton::LeftButton) {
    if (mouse_pos_ != MousePosition::NORMAL) {
      is_dragging_ = true;
      pos_start_ = event->pos();
      sz_start_ = QSize(this->geometry().width(), this->geometry().height());
    }
  }
}

void ImagePlayer::mouseReleaseEvent(QMouseEvent *event) {
  if (event->button() == Qt::MouseButton::LeftButton) {
    is_dragging_ = false;
  }
}

void ImagePlayer::mouseMoveEvent(QMouseEvent *event) {
  auto global_data = Singleton<GlobalData>::get();
  if (is_dragging_) {
    const auto diff = event->pos() - pos_start_;
    const auto current_item = images_.find(cb_channel_->currentText().toStdString());
    const float ratio = (current_item == images_.end()
                             ? 16.0f / 9
                             : current_item->second.cols * 1.0f / current_item->second.rows);
    QSize new_size;
    switch (mouse_pos_) {
      case MousePosition::RIGHT_BOTTOM:
      case MousePosition::RIGHT: {
        int new_width = sz_start_.width() + diff.x();
        new_width = CLAMP(new_width, 200, global_data->glwidget_->width());
        new_size.setWidth(new_width);
        new_size.setHeight(new_width / ratio);
        break;
      }

      case MousePosition::BOTTOM: {
        int new_height = sz_start_.height() + diff.y();
        new_height = CLAMP(new_height, 100, global_data->glwidget_->height());
        new_size.setHeight(new_height);
        new_size.setWidth(new_height * ratio);
        break;
      }

      default:;
    }
    this->setFixedSize(new_size);
    cb_channel_->setFixedWidth(new_size.width() / 2);
    cb_marker_->setGeometry(new_size.width() / 2, 0, std::min(new_size.width() / 2, 70), 20);
  } else {
    bool near_right = false, near_bottom = false;
    if (std::abs(event->pos().x() - this->geometry().width()) < 10) {
      near_right = true;
    }
    if (std::abs(event->pos().y() - this->geometry().height()) < 10) {
      near_bottom = true;
    }

    if (near_right && near_bottom) {
      this->setCursor(Qt::SizeFDiagCursor);
      mouse_pos_ = MousePosition::RIGHT_BOTTOM;
    } else if (near_right) {
      this->setCursor(Qt::SizeHorCursor);
      mouse_pos_ = MousePosition::RIGHT;
    } else if (near_bottom) {
      this->setCursor(Qt::SizeVerCursor);
      mouse_pos_ = MousePosition::BOTTOM;
    } else {
      this->setCursor(Qt::ArrowCursor);
      mouse_pos_ = MousePosition::NORMAL;
    }
  }
}

}  // namespace airi
}  // namespace crdc
