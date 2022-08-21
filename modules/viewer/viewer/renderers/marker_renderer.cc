#include "viewer/renderers/marker_renderer.h"

#include "viewer/global_data.h"
#include "viewer/renderer_manager.h"
#include "viewer/widgets/renderer_item.h"

namespace crdc {
namespace airi {

class MarkerChannel : public Renderer {
 public:
  MarkerChannel(const std::string &channel, RendererItem *renderer_item) : channel_(channel) {
    item_ = new RendererItem(QString::fromStdString(channel), enabled(),
                                 [&](bool is_checked) {
      auto channel_enable = global_data_->config_.mutable_marker_channel_enable();
      (*channel_enable)[channel_] = is_checked;
    });
    renderer_item->addWidget(item_);
  }

  std::string name() const override { return "MarkerRenderer/" + channel_; }

  bool enabled() const override {
    auto it = global_data_->config_.marker_channel_enable().find(channel_);
    return (it != global_data_->config_.marker_channel_enable().end() && (it->second));
  }

  void render() override {
    if (!msg_) {
      return;
    }

    for (const auto &marker : msg_->markers()) {
      if (!marker.on_off()) {
        continue;
      }

      // push guard
      GLPushGuard pg;

      // coordinate transform
      // if (marker.has_frame_id() && marker.frame_id() != "global") {
      //   transform(marker.frame_id(), "global", marker.utime());
      // }

      // set color
      if (marker.has_color()) {
        if (marker.color().has_color3b()) {
          const auto &color = marker.color().color3b();
          glColor3ub(color.r(), color.g(), color.b());
        } else if (marker.color().has_color3f()) {
          const auto &color = marker.color().color3f();
          glColor3f(color.r(), color.g(), color.b());
        } else if (marker.color().has_color4f()) {
          const auto &color = marker.color().color4f();
          glColor4f(color.r(), color.g(), color.b(), color.a());
        }
      }

      // set point size
      if (marker.has_point_size()) {
        glPointSize(marker.point_size());
      }

      // set line width
      if (marker.has_line_width()) {
        glLineWidth(marker.line_width());
      }

      // draw according to marker type
      if (marker.has_points()) {
        if (marker.points().points_size() == 0) {
          continue;
        }
        std::vector<Eigen::VectorXf> points;
        points.reserve(marker.points().points_size());
        for (const auto &pt : marker.points().points()) {
          points.push_back(Eigen::Vector3f(pt.x(), pt.y(), pt.z()));
        }
        drawPoints(points);
      } else if (marker.has_lines()) {
        const int num_pairs = marker.lines().points_size() / 2;
        if (num_pairs == 0) {
          continue;
        }
        std::vector<Eigen::VectorXf> points;
        points.reserve(marker.lines().points_size());
        for (int i = 0; i < num_pairs * 2; ++i) {
          const auto &pt = marker.lines().points(i);
          points.push_back(Eigen::Vector3f(pt.x(), pt.y(), pt.z()));
        }
        drawLines(points);
      } else if (marker.has_line_strip()) {
        if (marker.line_strip().points_size() < 2) {
          continue;
        }
        std::vector<Eigen::VectorXf> points;
        points.reserve(marker.line_strip().points_size());
        for (const auto &pt : marker.line_strip().points()) {
          points.push_back(Eigen::Vector3f(pt.x(), pt.y(), pt.z()));
        }
        drawLineStrip(points);
      } else if (marker.has_arrow()) {
        const auto &from = marker.arrow().from();
        const auto &to = marker.arrow().to();
        drawArrow(Eigen::Vector3f(from.x(), from.y(), from.z()),
                  Eigen::Vector3f(to.x(), to.y(), to.z()), marker.arrow().scale());
      } else if (marker.has_triangles()) {
        const int num_tripples = marker.triangles().points_size() / 3;
        if (num_tripples == 0) {
          continue;
        }
        std::vector<Eigen::VectorXf> points;
        points.reserve(marker.triangles().points_size());
        for (int i = 0; i < num_tripples * 3; ++i) {
          const auto &pt = marker.triangles().points(i);
          points.push_back(Eigen::Vector3f(pt.x(), pt.y(), pt.z()));
        }
        drawTriangles(points);
      } else if (marker.has_sphere()) {
        const auto &center = marker.sphere().center();
        drawSphere(Eigen::Vector3f(center.x(), center.y(), center.z()), marker.sphere().radius());
      } else if (marker.has_cube()) {
        const auto &cube = marker.cube();
        const auto &center = cube.center();
        drawBoundingBox(Eigen::Vector3f(center.x(), center.y(), center.z()),
                        Eigen::Vector3f(cube.length(), cube.width(), cube.height()), cube.heading(),
                        true, false);
      } else if (marker.has_polygon()) {
        if (marker.polygon().point_size() < 3) {
          continue;
        }
        std::vector<Eigen::Vector2f> points;
        points.reserve(marker.polygon().point_size());
        for (int i = 0; i < marker.polygon().point_size(); ++i) {
          const auto &pt = marker.polygon().point(i);
          points.emplace_back(pt.x(), pt.y());
        }

        // GLPushGuard pg;
        // if (marker.polygon().point().has_z()) {
        //   glTranslatef(0, 0, marker.polygon().point().z());
        // }
        // drawPolygon(points);
        std::vector<std::vector<Eigen::Vector2f>> polygons;
        polygons.push_back(std::move(points));
        auto buffer = generateGLBuffer(polygons);
        drawElements(GL_TRIANGLES, buffer);
      } else if (marker.has_text()) {
        const auto &pos = marker.text().position();
        drawText(Eigen::Vector3f(pos.x(), pos.y(), pos.z()), marker.text().text(),
                 marker.text().font_size(), marker.text().bold());
      }
    }
  }

  void loadConfigPost() override {
    item_->setChecked(enabled());
  }

  void update(const std::shared_ptr<MarkerList> &msg) { msg_ = msg; }

 protected:
  RendererItem *item_;
  const std::string channel_;
  std::shared_ptr<MarkerList> msg_;
};

MarkerRenderer::MarkerRenderer() {
  item_ = new RendererItem(
      "Marker", global_data_->config_.marker_renderer_enable(),
      [&](bool is_checked) { global_data_->config_.set_marker_renderer_enable(is_checked); });
  global_data_->renderer_manager_->addWidget(item_);
}

bool MarkerRenderer::enabled() const { return global_data_->config_.marker_renderer_enable(); }

void MarkerRenderer::initialize() {
  Renderer::initialize();

  global_data_->message_hub_->subscribe<MarkerList>(
      [&](const std::string &channel, const std::shared_ptr<MarkerList> &msg) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (channels_.find(channel) == channels_.end()) {
          to_be_added_.insert(channel);
        }
        msgs_[channel] = msg;
        needs_update_[channel] = true;
      });
}

void MarkerRenderer::render() {
  std::unique_lock<std::mutex> lock(mutex_);

  // update widgets
  for (auto it = to_be_added_.begin(); it != to_be_added_.end();) {
    channels_[*it].reset(new MarkerChannel(*it, item_));
    channels_[*it]->initialize();
    it = to_be_added_.erase(it);
  }

  if (!enabled()) {
    return;
  }

  // update messages
  for (auto &channel : channels_) {
    if (channel.second->enabled()) {
      if (needs_update_[channel.first]) {
        needs_update_[channel.first] = false;
        channel.second->update(msgs_[channel.first]);
      }
    }
  }

  lock.unlock();

  // render
  for (auto &channel : channels_) {
    if (channel.second->enabled()) {
      channel.second->render();
    }
  }
}

void MarkerRenderer::loadConfigPost() {
  item_->setChecked(enabled());

  for (auto &channel : channels_) {
    channel.second->loadConfigPost();
  }
}

}  // namespace airi
}  // namespace crdc
