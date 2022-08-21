#include "viewer/renderers/texturemap_renderer.h"
#include <functional>
#include <thread>
#include "cyber/common/log.h"
#include "common/io/file.h"
#include "viewer/global_data.h"
#include "viewer/renderer_manager.h"
#include "viewer/widgets/check_box.h"
#include "viewer/widgets/renderer_item.h"
#include "viewer/widgets/slider.h"

namespace crdc {
namespace airi {

TexturemapRenderer::TexturemapRenderer() {
  auto item = new RendererItem(
      "Texture Map", global_data_->config_.texturemap_renderer_enable(),
      [&](bool is_checked) { global_data_->config_.set_texturemap_renderer_enable(is_checked); });
  global_data_->renderer_manager_->addWidget(item);

  for (const auto &info : global_data_->config_.texturemaps()) {
    const auto &name = info.name();
    names_.insert(name);
    loading_[name].store(false);
    loaded_[name].store(false);
    prepared_[name].store(false);
    enables_[name].store(false);
    alpha_[name] = .5f;

    auto cb_enable =
        new CheckBox(QString::fromStdString(name), enables_[name], [=](bool is_checked) {
          enables_[name].store(is_checked);

          if (is_checked && !loaded_[name].load() && !loading_[name].load()) {
            std::thread handle_thread_load(
                std::bind(&TexturemapRenderer::threadLoadTextureDesc, this, info));
            handle_thread_load.detach();
          }
        });
    item->addWidget(cb_enable);

    auto slider_alpha =
        new Slider("Alpha", 2, 0, 1, alpha_[name], [&](double val) { alpha_[name] = val; });
    item->addWidget(slider_alpha);
  }
}

bool TexturemapRenderer::enabled() const {
  return global_data_->config_.texturemap_renderer_enable();
}

void TexturemapRenderer::render() {
  if (!enabled()) {
    return;
  }

  for (const auto &name : names_) {
    if (!enables_[name].load() || !loaded_[name].load()) {
      continue;
    }

    if (!prepared_[name].load()) {
      LOG(INFO) << "Preparing texturemap for " << name;
      for (const auto &desc : texture_descs_[name]) {
        auto qimg = QImage(desc.image_.data, desc.image_.cols, desc.image_.rows, desc.image_.step,
                           QImage::Format_RGB888);
        GLTexture texture;
        texture.texture_.reset(new QOpenGLTexture(qimg));
        texture.roi_ = desc.roi_;
        textures_[name].push_back(texture);
      }
      LOG(INFO) << "Prepared texturemap for " << name;
      texture_descs_[name].clear();
      prepared_[name].store(true);
    }

    glColor4f(1, 1, 1, alpha_[name]);
    for (const auto &texture : textures_[name]) {
      renderTexture(texture, {offset_[name].x(), offset_[name].y()}, resolution_[name]);
    }
  }
}

void TexturemapRenderer::threadLoadTextureDesc(const viewer::TexturemapInfo &info) {
  loading_[info.name()].store(true);
  LOG(INFO) << "Loading texturemap for " << info.name();

  texture_descs_[info.name()] =
      loadTextureDesc(crdc::airi::util::get_absolute_path(std::getenv("CRDC_WS"), info.path()));
  offset_[info.name()] = Eigen::Vector2f(info.offset_x(), info.offset_y());
  resolution_[info.name()] = info.resolution();

  LOG(INFO) << "Loaded texturemap for " << info.name();
  loaded_[info.name()].store(true);
  loading_[info.name()].store(false);
}

}  // namespace airi
}  // namespace crdc
