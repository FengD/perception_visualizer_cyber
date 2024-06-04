#include "viewer/global_data.h"
#include <FTGL/ftgl.h>
#include <gflags/gflags.h>
#include <random>
#include "common/io/file.h"
#include "viewer/message/message_hub.h"

#include "cyber/sensor_proto/marker.pb.h"
#include "cyber/sensor_proto/image_marker.pb.h"

DEFINE_string(config, "params/viewer/config.prototxt", "path of config prototxt");
DEFINE_bool(mock, false, "enable_mock");

using crdc::airi::LocalizationEstimate;

namespace crdc {
namespace airi {

void GlobalData::initialize() {
  // load config
  const auto &path_config = crdc::airi::util::get_absolute_path(std::getenv("CRDC_WS"), FLAGS_config);
    if(!crdc::airi::util::get_proto_from_file(path_config, &config_)) {
      LOG(ERROR) << "Failed to load config file: " << path_config;
    }

  // load font
  const auto &path_font_normal =
      crdc::airi::util::get_absolute_path(std::getenv("CRDC_WS"), config_.path_font_normal());
  font_normal_.reset(new FTGLPixmapFont(path_font_normal.c_str()));
  if (font_normal_->Error()) {
    LOG(ERROR) << "Failed to load normal font: " << path_font_normal;
    font_normal_.reset();
  }
  const auto &path_font_bold =
      crdc::airi::util::get_absolute_path(std::getenv("CRDC_WS"), config_.path_font_bold());
  font_bold_.reset(new FTGLPixmapFont(path_font_bold.c_str()));
  if (font_bold_->Error()) {
    LOG(ERROR) << "Failed to load bold font: " << path_font_bold;
    font_bold_.reset();
  }

  // init message hub
  message_hub_.reset(new MessageHub());

  // init pose
  pose_ = std::make_shared<LocalizationEstimate>();
  auto pose = pose_->mutable_pose();
  auto pos = pose->mutable_position();
  pos->set_x(0);
  pos->set_y(0);
  auto ori = pose->mutable_orientation();
  ori->set_qw(1);
  ori->set_qx(0);
  ori->set_qy(0);
  ori->set_qz(0);
}

std::shared_ptr<crdc::airi::LocalizationEstimate> GlobalData::pose() {
  std::lock_guard<std::mutex> lock(mutex_pose_);
  return pose_;
}

void GlobalData::threadMock() {
  while (enable_thread_mock_.load()) {
    auto image_markers = std::make_shared<ImageMarkerList>();
    image_markers->set_image_channel("CAM_HK_FS0_L");
    auto image_marker = image_markers->add_markers();
    image_marker->set_size(3);
    auto cc = image_marker->mutable_color();
    auto c3b = cc->mutable_color3b();
    c3b->set_b(0);
    c3b->set_g(0);
    c3b->set_r(255);
    auto tt = image_marker->mutable_text();
    tt->set_scale(3);
    tt->set_text("Hello, world!");
    auto pp = tt->mutable_pos();
    pp->set_x(100);
    pp->set_y(200);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
}

}  // namespace airi
}  // namespace crdc
