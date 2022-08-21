#pragma once

#include <unordered_map>
#include "viewer/renderers/renderer.h"

namespace crdc {
namespace airi {

class RendererItem;

class FrameRenderer : public Renderer {
 public:
  FrameRenderer();

 public:
  std::string name() const override { return "FrameRenderer"; }

  bool enabled() const override;

  void render() override;

  void loadConfigPost() override;

 protected:
  RendererItem *item_;
  std::unordered_map<std::string, bool> enables_;
};

}  // namespace airi
}  // namespace crdc
