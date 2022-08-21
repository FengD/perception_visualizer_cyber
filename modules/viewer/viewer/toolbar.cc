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
#include <Eigen/Eigen>

namespace crdc {
namespace airi {

Toolbar::Toolbar() {
  this->setFocusPolicy(Qt::NoFocus);
  const int height = 42;
  const QSize icon_size(height * 2 / 3, height * 2 / 3);
  this->setMaximumHeight(height);

  auto global_data = Singleton<GlobalData>::get();

  // button: info
  auto bt_info = new PushButton(QIcon(std::getenv("CRDC_WS") + QString("/icons/info.png")), [=]() {
    auto info = new Info();
    info->show();
  });
  bt_info->setToolTip("Information");
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

  // button: follow
  auto bt_follow = new ToggleButton(
      QIcon(std::getenv("CRDC_WS") + QString("/icons/camera.png")), false, [=](bool is_checked) {
        if (is_checked) {
          pose_prev_ = global_data->pose();

          global_data->camera_->setCameraControlCallback([=]() {
            auto pose = global_data->pose();
            const auto &pos_prev = pose_prev_->pose().position();
            const auto &ori_prev = pose_prev_->pose().orientation();
            const auto &pos = pose->pose().position();
            const auto &ori = pose->pose().orientation();
            global_data->camera_->rotateAndTranslate(
                glm::dvec3(pos_prev.x(), pos_prev.y(), 0), glm::dvec3(pos.x(), pos.y(), 0),
                glm::dquat(ori_prev.qw(), ori_prev.qx(), ori_prev.qy(), ori_prev.qz()),
                glm::dquat(ori.qw(), ori.qx(), ori.qy(), ori.qz()));
            pose_prev_ = pose;
          });
        } else {
          global_data->camera_->setCameraControlCallback([] {});
        }
      });
  bt_follow->click();
  bt_follow->setToolTip("Camera Follow");
  bt_follow->setMaximumHeight(height);
  bt_follow->setFixedWidth(height);
  bt_follow->setIconSize(icon_size);

  // button: image player
  auto bt_image_player = new ToggleButton(
      QIcon(std::getenv("CRDC_WS") + QString("/icons/video.png")), false,
      [=](bool is_checked) { global_data->image_player_->setHidden(!is_checked); });
  bt_image_player->setToolTip("Image Player");
  bt_image_player->setMaximumHeight(height);
  bt_image_player->setFixedWidth(height);
  bt_image_player->setIconSize(icon_size);

  // button: measure
  global_data->bt_measure_ = new ToggleButton(QIcon(std::getenv("CRDC_WS") + QString("/icons/ruler.png")), false,
      [=](bool is_checked) {  });
  global_data->bt_measure_->setToolTip("Measure");
  global_data->bt_measure_->setText("Measure");
  global_data->bt_measure_->setMaximumHeight(height);
  global_data->bt_measure_->setIconSize(icon_size);

  // button: screenshot
  auto bt_screenshot =
      new PushButton(QIcon(std::getenv("CRDC_WS") + QString("/icons/screenshot.png")), [=]() {
        auto qimg = global_data->glwidget_->grabFrameBuffer();
        cv::Mat img(qimg.height(), qimg.width(), CV_8UC4, qimg.bits());
        cv::cvtColor(img, img, CV_BGRA2BGR);
        const auto path = std::getenv("CRDC_WS") + std::string("/daydream_screenshot_") +
                          std::to_string(apollo::cyber::Time::Now().ToNanosecond() / 1000) + ".png";
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
          const auto path = std::getenv("CRDC_WS") + std::string("/daydream_record_") +
                            std::to_string(apollo::cyber::Time::Now().ToNanosecond() / 1000) +
                            ".avi";
          vw.open(path, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'), 15.0,
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

  // button: chart widget
  // auto bt_chart_widget = new ToggleButton(
  //     QIcon(std::getenv("CRDC_WS") + QString("/icons/chart.png")), false,
  //     [=](bool is_checked) { global_data->chart_widget_->setHidden(!is_checked); });
  // bt_chart_widget->setToolTip("Chart Widget");
  // bt_chart_widget->setMaximumHeight(height);
  // bt_chart_widget->setFixedWidth(height);
  // bt_chart_widget->setIconSize(icon_size);

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
  left->addWidget(bt_follow);
  left->addWidget(bt_image_player);
  left->addWidget(global_data->bt_measure_);
  right->addWidget(bt_screenshot);
  right->addWidget(bt_record);
  // right->addWidget(bt_chart_widget);
  right->addWidget(bt_renderer_manager);
}

}  // namespace airi
}  // namespace crdc
