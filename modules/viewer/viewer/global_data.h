#pragma once

#include <QPoint>
#include <atomic>
#include <memory>
#include <mutex>
#include <thread>
#include "viewer/proto/config.pb.h"
#include "viewer/singleton.h"
#include "cyber/sensor_proto/localization.pb.h"
// #include "tf2/tf2.h"
#include "viewer/renderers/renderer.h"
#include <QPushButton>
#include <unordered_map>
#include "cyber/sensor_proto/pnc_point.pb.h"
#include "viewer/glwidget.h"

class FTFont;

// namespace tf2 {
// extern void transformTF2ToMsg(const tf2::Transform &tf2, geometry_msgs::Transform &msg);
// extern void transformMsgToTF2(const geometry_msgs::Transform &msg, tf2::Transform &tf2);
// }  // namespace tf2

namespace crdc {
namespace airi {

class GLWidget;
class MessageHub;
class Camera;
// class ChartWidget;
class RendererManager;
class Toolbar;
class ImagePlayer;
class MainWindow;

class GlobalData {
 public:
  void initialize();

 public:
  std::shared_ptr<crdc::airi::LocalizationEstimate> pose();

 protected:
  // void onLocalization(const std::string &channel,
  //                     const std::shared_ptr<crdc::airi::LocalizationEstimate> &msg);

  void threadMock();

 public:
  viewer::Config config_;
  // tf2::extend::TF *tf_;
  std::shared_ptr<MessageHub> message_hub_;

 public:
  std::shared_ptr<FTFont> font_normal_;
  std::shared_ptr<FTFont> font_bold_;
  std::shared_ptr<Camera> camera_;
  std::unordered_map<std::string, GLTexture> textures_;
  GLWidget *glwidget_;
  // ChartWidget *chart_widget_;
  RendererManager *renderer_manager_;
  Toolbar *toolbar_;
  ImagePlayer *image_player_;
  MainWindow *main_window_;

 public:
  bool measuring_{false};
  QPushButton *bt_measure_;
  QPoint pt_mouse_press_;
  QPoint pt_mouse_current_;
  QPointF pt3d_mouse_press_;

 public:
  std::vector<crdc::airi::PointENU> routing_points_;
  // std::unordered_map<std::string, std::shared_ptr<apollo::cyber::Writer<crdc::airi::PathPoint>>> writers_path_point_;

 protected:
  std::shared_ptr<crdc::airi::LocalizationEstimate> pose_;
  std::mutex mutex_pose_;

  std::atomic<bool> enable_thread_mock_;
  std::unique_ptr<std::thread> handle_thread_mock_;

 private:
  MAKE_SINGLETON(GlobalData);
};

}  // namespace airi
}  // namespace crdc
