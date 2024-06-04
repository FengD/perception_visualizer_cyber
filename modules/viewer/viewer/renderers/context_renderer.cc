#include "viewer/renderers/context_renderer.h"
#include "viewer/camera.h"
#include "viewer/global_data.h"
#include "viewer/renderer_manager.h"
#include "viewer/widgets/check_box.h"
#include "viewer/widgets/renderer_item.h"
#include "viewer/widgets/slider.h"
#include <QMatrix4x4>

namespace crdc {
namespace airi {

class GLWidget;

ContextRenderer::ContextRenderer() {
  item_ = new RendererItem(
      "Context", global_data_->config_.context_renderer_enable(),
      [&](bool is_checked) { global_data_->config_.set_context_renderer_enable(is_checked); });
  global_data_->renderer_manager_->addWidget(item_);

  auto cb_grid_enable = new CheckBox(
      "Show Grid", global_data_->config_.context_grid_enable(),
      [&](bool is_checked) { global_data_->config_.set_context_grid_enable(is_checked); });
  item_->addWidget(cb_grid_enable);

  auto slider_grid_range =
      new Slider("Grid Range", 1, 10, 5000, global_data_->config_.context_grid_range(),
                 [&](double val) { global_data_->config_.set_context_grid_range(val); });
  item_->addWidget(slider_grid_range);

  auto slider_grid_size =
      new Slider("Grid Size", 1, 0.1, 100, global_data_->config_.context_grid_size(),
                 [&](double val) { global_data_->config_.set_context_grid_size(val); });
  item_->addWidget(slider_grid_size);

  auto hbox = new QHBoxLayout();
  auto label = new QLabel("Center Frame: ");
  cb_grid_frame_id_= new QComboBox();
  cb_grid_frame_id_->setFocusPolicy(Qt::NoFocus);
  cb_grid_frame_id_->addItem("global");
  // cb_grid_frame_id_->addItem("local");
  // cb_grid_frame_id_->addItem("wheel");
  // cb_grid_frame_id_->addItem("body");
  // cb_grid_frame_id_->addItem("BASE_LIDAR");
  // cb_grid_frame_id_->addItem("ROBO_LEFT");
  // cb_grid_frame_id_->addItem("ROBO_RIGHT");
  // cb_grid_frame_id_->addItem("ROBO_FRONT");
  // cb_grid_frame_id_->addItem("ROBO_BACK");
  // cb_grid_frame_id_->addItem("CAR_HEAD");
  hbox->addWidget(label);
  hbox->addWidget(cb_grid_frame_id_);
  item_->addLayout(hbox);
}

bool ContextRenderer::enabled() const { return global_data_->config_.context_renderer_enable(); }

void ContextRenderer::render() {
  if (!enabled()) {
    return;
  }

  if (cb_grid_frame_id_->currentText() == "global") {
    glTranslatef(0, 0, global_data_->pose()->pose().position().z());
  }
  else {
    // transform(cb_grid_frame_id_->currentText().toStdString());
  }

  if (global_data_->config_.context_grid_enable()) {
    const auto grid_size = global_data_->config_.context_grid_size();
    const auto grid_range = global_data_->config_.context_grid_range();
    const auto grid_color = global_data_->config_.context_grid_color();
    const auto grid_line_width = global_data_->config_.context_grid_line_width();

    std::vector<Eigen::VectorXf> points;
    for (float x = -grid_range; x <= grid_range; x += grid_size) {
      points.push_back(Eigen::Vector2f(x, -grid_range));
      points.push_back(Eigen::Vector2f(x, grid_range));
    }
    for (float y = -grid_range; y <= grid_range; y += grid_size) {
      points.push_back(Eigen::Vector2f(-grid_range, y));
      points.push_back(Eigen::Vector2f(grid_range, y));
    }
#ifdef __aarch64__
    global_data_->glwidget_->setColor(QVector4D(grid_color.r(), grid_color.g(), grid_color.b(), grid_color.a()));
#else
    glColor4f(grid_color.r(), grid_color.g(), grid_color.b(), grid_color.a());
#endif
    glLineWidth(grid_line_width);
    drawLines(points);
  }
}

void ContextRenderer::loadConfigPost() {
  item_->setChecked(enabled());
}

}  // namespace airi
}  // namespace crdc
