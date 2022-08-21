#pragma once

#include <atomic>
#include <set>
#include <unordered_map>
#include "viewer/proto/config.pb.h"
#include "viewer/renderers/renderer.h"

namespace crdc {
namespace airi {

class TexturemapRenderer : public Renderer {
 public:
  TexturemapRenderer();

 public:
  std::string name() const override { return "TexturemapRenderer"; }

  bool enabled() const override;

  void render() override;

 protected:
  void threadLoadTextureDesc(const viewer::TexturemapInfo &info);

 protected:
  std::set<std::string> names_;
  std::unordered_map<std::string, std::atomic<bool>> loading_;
  std::unordered_map<std::string, std::atomic<bool>> loaded_;
  std::unordered_map<std::string, std::atomic<bool>> prepared_;
  std::unordered_map<std::string, std::atomic<bool>> enables_;
  std::unordered_map<std::string, float> alpha_;
  std::unordered_map<std::string, Eigen::Vector2f> offset_;
  std::unordered_map<std::string, float> resolution_;
  std::unordered_map<std::string, std::vector<GLTextureDesc>> texture_descs_;
  std::unordered_map<std::string, std::vector<GLTexture>> textures_;
};

}  // namespace airi
}  // namespace crdc
