#pragma once

#include <QPoint>
#include <atomic>
#include <memory>
#include <mutex>
#include <thread>
#include "viewer/proto/config.pb.h"
#include "common/common.h"
#include "cyber/sensor_proto/localization.pb.h"
#include "viewer/renderers/renderer.h"
#include <QPushButton>
#include <unordered_map>
#include "viewer/glwidget.h"

class FTFont;


namespace crdc {
namespace airi {

class GLWidget;
class MessageHub;
class Camera;
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

  void threadMock();

 public:
  viewer::Config config_;
  std::shared_ptr<MessageHub> message_hub_;

 public:
  std::shared_ptr<FTFont> font_normal_;
  std::shared_ptr<FTFont> font_bold_;
  std::shared_ptr<Camera> camera_;
  std::unordered_map<std::string, GLTexture> textures_;
  GLWidget *glwidget_;
  RendererManager *renderer_manager_;
  Toolbar *toolbar_;
  size_t ip_counter_;
  std::vector<ImagePlayer*> image_players_;
  bool is_mining_ = false;
  MainWindow *main_window_;

 public:
  bool measuring_{false};
  QPushButton *bt_measure_;
  QPoint pt_mouse_press_;
  QPoint pt_mouse_current_;
  QPointF pt3d_mouse_press_;

 public:
  std::vector<crdc::airi::PointENU> routing_points_;

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
