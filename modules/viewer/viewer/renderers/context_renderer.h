#pragma once

#include "viewer/renderers/renderer.h"
#include <QComboBox>

namespace crdc {
namespace airi {

class RendererItem;

class ContextRenderer : public Renderer {
 public:
  ContextRenderer();

 public:
  std::string name() const override { return "ContextRenderer"; }

  bool enabled() const override;

  void render() override;

  void loadConfigPost() override;

 protected:
  RendererItem *item_;
  QComboBox *cb_grid_frame_id_;
};

}  // namespace airi
}  // namespace crdc
