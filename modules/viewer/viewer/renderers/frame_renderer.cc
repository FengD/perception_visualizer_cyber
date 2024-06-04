#include "viewer/renderers/frame_renderer.h"
#include "viewer/global_data.h"
#include "viewer/renderer_manager.h"
#include "viewer/widgets/check_box.h"
#include "viewer/widgets/renderer_item.h"

namespace crdc {
namespace airi {

FrameRenderer::FrameRenderer() {
  item_ = new RendererItem(
      "Frame", global_data_->config_.frame_renderer_enable(),
      [&](bool is_checked) { global_data_->config_.set_frame_renderer_enable(is_checked); });
  global_data_->renderer_manager_->addWidget(item_);

  for (const auto &frame : global_data_->config_.frame_renderer_frames()) {
    enables_[frame] = false;
    auto cb_global = new CheckBox(QString::fromStdString(frame), enables_[frame],
                                  [&](bool is_checked) { enables_[frame] = is_checked; });
    item_->addWidget(cb_global);
  }
}

bool FrameRenderer::enabled() const { return global_data_->config_.frame_renderer_enable(); }

void FrameRenderer::render() {
  if (!enabled()) {
    return;
  }

  for (const auto &frame : global_data_->config_.frame_renderer_frames()) {
    if (!enables_[frame]) {
      continue;
    }

    GLPushGuard pg;

    glLineWidth(global_data_->config_.frame_line_width());
    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);

    glColor4f(1, 0, 0, 1);
    glBegin(GL_LINES);
    glVertex3f(0, 0, 0);
    glVertex3f(global_data_->config_.frame_line_length(), 0, 0);
    glEnd();
    drawText(Eigen::Vector3f(global_data_->config_.frame_line_length(), 0, 0), "X",
             global_data_->config_.frame_font_size(), global_data_->config_.frame_font_bold());

    glColor4f(0, 1, 0, 1);
    glBegin(GL_LINES);
    glVertex3f(0, 0, 0);
    glVertex3f(0, global_data_->config_.frame_line_length(), 0);
    glEnd();
    drawText(Eigen::Vector3f(0, global_data_->config_.frame_line_length(), 0), "Y",
             global_data_->config_.frame_font_size(), global_data_->config_.frame_font_bold());

    glColor4f(0, 0, 1, 1);
    glBegin(GL_LINES);
    glVertex3f(0, 0, 0);
    glVertex3f(0, 0, global_data_->config_.frame_line_length());
    glEnd();
    drawText(Eigen::Vector3f(0, 0, global_data_->config_.frame_line_length()), "Z",
             global_data_->config_.frame_font_size(), global_data_->config_.frame_font_bold());

    glColor4f(1, 1, 1, 1);
    drawText(Eigen::Vector3f(0, 0, 0), frame, global_data_->config_.frame_font_size(),
             global_data_->config_.frame_font_bold());

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
  }
}

void FrameRenderer::loadConfigPost() {
  item_->setChecked(enabled());
}

}  // namespace airi
}  // namespace crdc
