#include "viewer/renderers/view_renderer.h"
#include "viewer/global_data.h"
#include "viewer/renderer_manager.h"
#include "viewer/camera.h"
#include "viewer/widgets/renderer_item.h"
#include "viewer/widgets/check_box.h"
#include "viewer/widgets/toggle_button.h"

namespace crdc {
namespace airi {

ViewRenderer::ViewRenderer() {
  auto item = new RendererItem("View", true, [&](bool is_checked) {});
  global_data_->renderer_manager_->addWidget(item);

  auto cb_lock_birdview = new CheckBox("Lock Birdview", false, [&](bool is_checked) {
    global_data_->camera_->lockBirdview(is_checked);
    if (is_checked) {
      auto pose = global_data_->pose();
      const auto &pos = pose->pose().position();
      const auto &ori = pose->pose().orientation();
      Eigen::Quaterniond q(ori.qw(), ori.qx(), ori.qy(), ori.qz());
      auto ypr = q.toRotationMatrix().eulerAngles(2, 1, 0);
      global_data_->camera_->reset();
      global_data_->camera_->jumpTo(glm::dvec3(pos.x(), pos.y(), 0), ypr[0] + M_PI_2);
    }
  });
  item->addWidget(cb_lock_birdview);
}

}  // namespace airi
}  // namespace crdc
