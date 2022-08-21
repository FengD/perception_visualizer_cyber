#pragma once

#include <mutex>
#include <set>
#include <unordered_map>
#include "viewer/message/message_hub.h"
#include "viewer/renderers/renderer.h"
#include "cyber/sensor_proto/marker.pb.h"

namespace crdc {
namespace airi {

class MarkerChannel;
class RendererItem;

class MarkerRenderer : public Renderer {
 public:
  MarkerRenderer();

 public:
  std::string name() const override { return "MarkerRenderer"; }

  bool enabled() const override;

  void initialize() override;

  void render() override;

  void loadConfigPost() override;

 protected:
  RendererItem *item_;
  std::unordered_map<std::string, std::shared_ptr<MarkerChannel>> channels_;
  std::unordered_map<std::string, std::shared_ptr<MarkerList>> msgs_;
  std::unordered_map<std::string, bool> needs_update_;
  std::set<std::string> to_be_added_;
  std::mutex mutex_;
};

}  // namespace airi
}  // namespace crdc
