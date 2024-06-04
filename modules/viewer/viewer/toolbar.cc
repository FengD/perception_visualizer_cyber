#include "viewer/toolbar.h"
#include <QIcon>
#include <QLayout>
#include <QTimer>
#include "viewer/camera.h"
#include "viewer/global_data.h"
#include "viewer/glwidget.h"
#include "viewer/image_player.h"
#include "viewer/info.h"
#include "viewer/renderer_manager.h"
#include "viewer/widgets/push_button.h"
#include "viewer/widgets/toggle_button.h"
#include "common/common.h"
#include <Eigen/Eigen>

namespace crdc {
namespace airi {

Toolbar::Toolbar() {
  this->setFocusPolicy(Qt::NoFocus);
  const int height = 42;
  const QSize icon_size(height * 2 / 3, height * 2 / 3);
  this->setMaximumHeight(height);

  auto global_data = crdc::airi::common::Singleton<GlobalData>::get();

  // button: info
  auto bt_info = new PushButton(QIcon(std::getenv("CRDC_WS") + QString("/icons/info.png")), [=]() {
    auto info = new Info();
    info->show();
  });
  bt_info->setToolTip("Tips");
  bt_info->setMaximumHeight(height);
  bt_info->setFixedWidth(height);
  bt_info->setIconSize(icon_size);

  // button: locate
  auto bt_locate = new PushButton(QIcon(std::getenv("CRDC_WS") + QString("/icons/locate.png")), [=]() {
    auto pose = global_data->pose();
    const auto &pos = pose->pose().position();
    const auto &ori = pose->pose().orientation();
    Eigen::Quaterniond q(ori.qw(), ori.qx(), ori.qy(), ori.qz());
    auto ypr = q.toRotationMatrix().eulerAngles(2, 1, 0);
    global_data->camera_->reset();
    global_data->camera_->jumpTo(glm::dvec3(pos.x(), pos.y(), 0), ypr[0] + M_PI_2);
  });
  bt_locate->setToolTip("Locate");
  bt_locate->setMaximumHeight(height);
  bt_locate->setFixedWidth(height);
  bt_locate->setIconSize(icon_size);

  // button: image player
  auto bt_image_player_add = new PushButton(QIcon(std::getenv("CRDC_WS") + QString("/icons/add.png")), [=]() {
    global_data->image_players_[global_data->ip_counter_]->setHidden(false);
    if (global_data->ip_counter_ < global_data->image_players_.size() - 1) {
      global_data->ip_counter_++;
    }
  });
  bt_image_player_add->setToolTip("Add Camera");
  bt_image_player_add->setMaximumHeight(height);
  bt_image_player_add->setFixedWidth(height);
  bt_image_player_add->setIconSize(icon_size);

  auto bt_image_player_del = new PushButton(QIcon(std::getenv("CRDC_WS") + QString("/icons/delete.png")), [=]() {
    global_data->image_players_[global_data->ip_counter_]->setHidden(true);
    if (global_data->ip_counter_ > 0) {
      global_data->ip_counter_--;
    }
  });
  bt_image_player_del->setToolTip("Remove Camera");
  bt_image_player_del->setMaximumHeight(height);
  bt_image_player_del->setFixedWidth(height);
  bt_image_player_del->setIconSize(icon_size);

  // button: measure
  global_data->bt_measure_ = new ToggleButton(QIcon(std::getenv("CRDC_WS") + QString("/icons/ruler.png")), false,
      [=](bool is_checked) {  });
  global_data->bt_measure_->setToolTip("Measure");
  global_data->bt_measure_->setText("Measure");
  global_data->bt_measure_->setMaximumHeight(height);
  global_data->bt_measure_->setIconSize(icon_size);

  // button: data mining
  auto data_mining_ =
      new PushButton(QIcon(std::getenv("CRDC_WS") + QString("/icons/export.png")), [=]() {
        global_data->is_mining_ = true;
      });
  data_mining_->setToolTip("Data Mining");
  data_mining_->setText("Data Mining");
  data_mining_->setMaximumHeight(height);
  data_mining_->setIconSize(icon_size);

  // button: screenshot
  auto bt_screenshot =
      new PushButton(QIcon(std::getenv("CRDC_WS") + QString("/icons/screenshot.png")), [=]() {
        auto qimg = global_data->glwidget_->grabFrameBuffer();
        cv::Mat img(qimg.height(), qimg.width(), CV_8UC4, qimg.bits());
        cv::cvtColor(img, img, CV_BGRA2BGR);
        const auto path = std::string("./viewer_screenshot_") +
                          std::to_string(get_now_microsecond() / 1000) + ".png";
        cv::imwrite(path, img);
      });
  bt_screenshot->setToolTip("Screenshot");
  bt_screenshot->setMaximumHeight(height);
  bt_screenshot->setFixedWidth(height);
  bt_screenshot->setIconSize(icon_size);

  // button: record
  static cv::VideoWriter vw;
  static QTimer timer;
  QObject::connect(&timer, &QTimer::timeout, [=]() {
    if (!vw.isOpened()) {
      return;
    }
    auto qimg = global_data->glwidget_->grabFrameBuffer();
    cv::Mat img(qimg.height(), qimg.width(), CV_8UC4, qimg.bits());
    cv::cvtColor(img, img, CV_BGRA2BGR);
    vw << img;
  });
  auto bt_record = new ToggleButton(
      QIcon(std::getenv("CRDC_WS") + QString("/icons/record.png")), false, [=](bool is_checked) {
        if (is_checked && !vw.isOpened()) {
          const auto path = std::string("./viewer_record_") +
                            std::to_string(get_now_microsecond() / 1000) +
                            ".avi";
          // set a lower fps for more clear visualization 
          vw.open(path, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'), 5.0,
                  cv::Size(global_data->glwidget_->width(), global_data->glwidget_->height()),
                  true);
          timer.start(66);
        } else if (!is_checked && vw.isOpened()) {
          timer.stop();
          vw.release();
        }
      });
  bt_record->setToolTip("Record Video");
  bt_record->setMaximumHeight(height);
  bt_record->setFixedWidth(height);
  bt_record->setIconSize(icon_size);

  // button: renderer manager
  auto bt_renderer_manager = new ToggleButton(
      QIcon(std::getenv("CRDC_WS") + QString("/icons/unfold.png")), true,
      [=](bool is_checked) { global_data->renderer_manager_->setHidden(!is_checked); });
  bt_renderer_manager->setToolTip("Renderer Manager");
  bt_renderer_manager->setMaximumHeight(height);
  bt_renderer_manager->setFixedWidth(height);
  bt_renderer_manager->setIconSize(icon_size);

  auto left = new QHBoxLayout();
  auto middle = new QHBoxLayout();
  auto right = new QHBoxLayout();
  auto layout = new QHBoxLayout();

  left->setAlignment(Qt::AlignLeft);
  middle->setAlignment(Qt::AlignCenter);
  right->setAlignment(Qt::AlignRight);
  layout->addLayout(left);
  layout->addLayout(middle);
  layout->addLayout(right);
  layout->setMargin(3);
  layout->setSpacing(3);
  this->setLayout(layout);

  left->addWidget(bt_info);
  left->addWidget(bt_locate);
  left->addWidget(bt_image_player_add);
  left->addWidget(bt_image_player_del);
  left->addWidget(global_data->bt_measure_);
  left->addWidget(data_mining_);
  right->addWidget(bt_screenshot);
  right->addWidget(bt_record);
  right->addWidget(bt_renderer_manager);
}

}  // namespace airi
}  // namespace crdc
