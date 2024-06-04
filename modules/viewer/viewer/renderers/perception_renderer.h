#pragma once

#include <mutex>
#include "viewer/message/message_hub.h"
#include "viewer/renderers/renderer.h"
#include "cyber/sensor_proto/perception_obstacle.pb.h"

namespace crdc {
namespace airi {

using crdc::airi::PerceptionObstacles;

class PerceptionChannel;
class RendererItem;

class PerceptionRenderer : public Renderer {
 public:
  PerceptionRenderer();

 public:
  std::string name() const override { return "PerceptionRenderer"; }

  bool enabled() const override;

  void initialize() override;

  void render() override;

  void loadConfigPost() override;

 protected:
  RendererItem *item_;
  std::unordered_map<std::string, std::shared_ptr<PerceptionChannel>> channels_;
  std::unordered_map<std::string, std::shared_ptr<PerceptionObstacles>> msgs_;
  std::unordered_map<std::string, bool> needs_update_;
  std::set<std::string> to_be_added_;
  std::mutex mutex_;
};

}  // namespace airi
}  // namespace crdc
