// #include "cyber/cyber.h"
#include "viewer/global_data.h"
#include <FTGL/ftgl.h>
#include <gflags/gflags.h>
#include <random>
#include "cyber/common/log.h"
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
  AERROR_IF(!crdc::airi::util::get_proto_from_file(path_config, &config_))
      << "Failed to load config file: " << path_config;

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

  // init tf
  // tf_ = &(tf2::extend::TF::instance());
  // CHECK(tf_);

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

  // subscribe localization messages
  // message_hub_->subscribe<LocalizationEstimate>(
  //     FLAGS_localization_topic,
  //     std::bind(&GlobalData::onLocalization, this, std::placeholders::_1, std::placeholders::_2));
  // message_hub_->subscribe<LocalizationEstimate>(
  //     FLAGS_localization_odom_topic,
  //     std::bind(&GlobalData::onLocalization, this, std::placeholders::_1, std::placeholders::_2));
  // message_hub_->subscribe<LocalizationEstimate>(
  //     FLAGS_localization_gpose_topic,
  //     std::bind(&GlobalData::onLocalization, this, std::placeholders::_1, std::placeholders::_2));

  // if (FLAGS_mock) {
  //   enable_thread_mock_.store(true);
  //   handle_thread_mock_.reset(new std::thread(std::bind(&GlobalData::threadMock, this)));
  // }
}

std::shared_ptr<crdc::airi::LocalizationEstimate> GlobalData::pose() {
  std::lock_guard<std::mutex> lock(mutex_pose_);
  return pose_;
}

// void GlobalData::onLocalization(const std::string &channel,
//                                 const std::shared_ptr<LocalizationEstimate> &msg) {
//   // update pose
//   if (channel == FLAGS_localization_topic) {
//     std::lock_guard<std::mutex> lock(mutex_pose_);
//     pose_ = msg;
//     return;
//   }

//   // update tf
//   geometry_msgs::TransformStamped transform;
//   if (channel == FLAGS_localization_odom_topic) {
//     transform.header.frame_id = "local";
//     transform.child_frame_id = "body";
//   }
//   else if(channel == FLAGS_localization_gpose_topic) {
//     transform.header.frame_id = "global";
//     transform.child_frame_id = "local";
//   }
//   else {
//     return;
//   }
//   transform.header.stamp = uint64_t(msg->header().timestamp_sec() * 1000000);
//   tf2::Vector3 trans(msg->pose().position().x(), msg->pose().position().y(),
//                      msg->pose().position().z());
//   tf2::Quaternion quat(msg->pose().orientation().qx(), msg->pose().orientation().qy(),
//                        msg->pose().orientation().qz(), msg->pose().orientation().qw());
//   tf2::transformTF2ToMsg(tf2::Transform(quat, trans), transform.transform);
//   AERROR_IF(!tf_->setTransform(transform, "viewer"))
//       << "Failed to set transform from body to global";
// }

void GlobalData::threadMock() {
  // auto node = message_hub_->node();

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
    // static auto writer_image_marker = node->CreateWriter<ImageMarkerList>("cao");
    // writer_image_marker->Write(image_markers);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
}

}  // namespace airi
}  // namespace crdc
