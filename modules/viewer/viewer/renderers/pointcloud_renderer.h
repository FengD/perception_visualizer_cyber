#pragma once

#include <mutex>
#include <set>
#include <unordered_map>
#include "viewer/message/message_hub.h"
#include "viewer/renderers/renderer.h"
#include "cyber/sensor_proto/lidar.pb.h"
#include <GL/gl.h>

namespace crdc {
namespace airi {

class PointCloudChannel;
class RendererItem;

class PointCloudRenderer : public Renderer {
 public:
  PointCloudRenderer();

 public:
  std::string name() const override { return "PointCloudRenderer"; }

  bool enabled() const override;

  void initialize() override;

  void render() override;

  void loadConfigPost() override;

 protected:
  RendererItem *item_;
  std::unordered_map<std::string, std::shared_ptr<PointCloudChannel>> channels_;
  std::unordered_map<std::string, std::shared_ptr<crdc::airi::PointCloud2>> to_be_added_;
  std::mutex mutex_;
};

}  // namespace airi
}  // namespace crdc
