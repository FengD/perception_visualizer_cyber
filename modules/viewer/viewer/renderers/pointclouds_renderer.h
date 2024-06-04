#pragma once

#include <mutex>
#include <set>
#include <unordered_map>
#include "viewer/message/message_hub.h"
#include "viewer/renderers/renderer.h"
#include "cyber/sensor_proto/lidar.pb.h"

namespace crdc {
namespace airi {

class PointCloudsChannel;
class RendererItem;

class PointCloudsRenderer : public Renderer {
 public:
  PointCloudsRenderer();

 public:
  std::string name() const override { return "PointCloudsRenderer"; }

  bool enabled() const override;

  void initialize() override;

  void render() override;

  void loadConfigPost() override;

 protected:
  RendererItem *item_;
  std::unordered_map<std::string, std::shared_ptr<PointCloudsChannel>> channels_;
  std::unordered_map<std::string, std::shared_ptr<crdc::airi::PointClouds2>>
      to_be_added_;
  std::mutex mutex_;
};

}  // namespace airi
}  // namespace crdc
