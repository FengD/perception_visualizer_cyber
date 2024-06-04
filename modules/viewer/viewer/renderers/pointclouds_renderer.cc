#include "viewer/renderers/pointclouds_renderer.h"
#include <QComboBox>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QRadioButton>
#include <boost/circular_buffer.hpp>
#include "viewer/camera.h"
#include "viewer/global_data.h"
#include "viewer/renderer_manager.h"
#include "viewer/widgets/check_box.h"
#include "viewer/widgets/color_button.h"
#include "viewer/widgets/renderer_item.h"
#include "viewer/widgets/slider.h"

namespace crdc {
namespace airi {

class PointCloudsChannel : public Renderer {
 public:
  PointCloudsChannel(const std::string &channel, RendererItem *renderer_item):
  channel_(channel) {
    item_ = new RendererItem(QString::fromStdString(channel), enabled(),
                             [&](bool is_checked) {
      auto channel_enable = global_data_->config_.mutable_pointclouds_channel_enable();
      (*channel_enable)[channel_] = is_checked;
    });
    renderer_item->addWidget(item_);

    // memory size
    vertexs_.set_capacity(1);
    buffers_.set_capacity(1);
    auto slider_memory_size = new Slider("Memory Size", 0, 1, 600, 1, [&](double val) {
      const size_t capacity = val;
      if (vertexs_.size() > capacity) {
        vertexs_.resize(capacity);
      }
      if (buffers_.size() > capacity) {
        buffers_.resize(capacity);
      }
      vertexs_.set_capacity(capacity);
      buffers_.set_capacity(capacity);
    });
    item_->addWidget(slider_memory_size);

    // point size
    auto slider_point_size = new Slider("Point Size", 0, 1., 100., point_size_,
                                        [&](double value) { point_size_ = float(value); });
    item_->addWidget(slider_point_size);

    // alpha
    auto slider_alpha =
        new Slider("Alpha", 1, 0., 1., alpha_, [&](double value) { alpha_ = float(value); });
    item_->addWidget(slider_alpha);

    // solid color
    auto hbox_solid = new QHBoxLayout();
    rb_solid_ = new QRadioButton("Solid: ");
    rb_solid_->setChecked(true);
    auto bt_color = new ColorButton(color_, [&](const QColor &color) { color_ = color; });
    hbox_solid->addWidget(rb_solid_);
    hbox_solid->addWidget(bt_color);
    item_->addLayout(hbox_solid);

    // colormap
    auto hbox_colormap = new QHBoxLayout();
    rb_colormap_ = new QRadioButton("Color: ");
    cb_render_type_ = new QComboBox();
    cb_render_type_->setFocusPolicy(Qt::NoFocus);
    cb_render_type_->addItem("AUTUMN", cv::COLORMAP_AUTUMN);
    cb_render_type_->addItem("BONE", cv::COLORMAP_BONE);
    cb_render_type_->addItem("JET", cv::COLORMAP_JET);
    cb_render_type_->addItem("WINTER", cv::COLORMAP_WINTER);
    cb_render_type_->addItem("RAINBOW", cv::COLORMAP_RAINBOW);
    cb_render_type_->addItem("OCEAN", cv::COLORMAP_OCEAN);
    cb_render_type_->addItem("SUMMER", cv::COLORMAP_SUMMER);
    cb_render_type_->addItem("SPRING", cv::COLORMAP_SPRING);
    cb_render_type_->addItem("COOL", cv::COLORMAP_COOL);
    cb_render_type_->addItem("HSV", cv::COLORMAP_HSV);
    cb_render_type_->addItem("PINK", cv::COLORMAP_PINK);
    cb_render_type_->addItem("HOT", cv::COLORMAP_HOT);
    cb_render_field_ = new QComboBox();
    cb_render_field_->setFocusPolicy(Qt::NoFocus);
    hbox_colormap->addWidget(rb_colormap_);
    hbox_colormap->addWidget(cb_render_type_);
    hbox_colormap->addWidget(cb_render_field_);
    item_->addLayout(hbox_colormap);

    // colormap range
    auto hbox_colormap_range = new QHBoxLayout();
    cb_render_range_auto_ = new CheckBox("Auto", false, [&](bool is_checked) {
      le_render_min_->setEnabled(!is_checked);
      le_render_max_->setEnabled(!is_checked);
    });
    hbox_colormap_range->addWidget(cb_render_range_auto_);
    le_render_min_ = new QLineEdit("Min");
    hbox_colormap_range->addWidget(le_render_min_);
    le_render_max_ = new QLineEdit("Max");
    hbox_colormap_range->addWidget(le_render_max_);
    cb_render_range_auto_->click();
    item_->addLayout(hbox_colormap_range);

    auto label_show_pointcloud = new QLabel("Show PointCloud:");
    item_->addWidget(label_show_pointcloud);
  }

 public:
  void loadConfigPost() override {
    item_->setChecked(enabled());
  }

  std::string name() const override { return "PointCloudsRenderer/" + channel_; }

  bool enabled() const {
    auto it = global_data_->config_.pointclouds_channel_enable().find(channel_);
    return (it != global_data_->config_.pointclouds_channel_enable().end() && (it->second));
  }

  void render() override {
    if (!initialized_) {
      return;
    }

    {
      std::lock_guard<std::mutex> lock(mutex_);
      if (needs_update_) {
        const size_t sz = buffers_.capacity();
        size_t i = 0;
        for (auto it = vertexs_.rbegin(); it != vertexs_.rend(); ++it) {
          if (i++ >= sz) {
            break;
          }

          GLBufferWithTrans bwt;
          bwt.frame_id = it->frame_id;
          bwt.utime = it->utime;

          if (it->vertex.rows() == 3) {
            bwt.buffer = generateGLBuffer(it->vertex, 3, 0);
          } else {
            bwt.buffer = generateGLBuffer(it->vertex, 3, 4);
          }

          buffers_.push_back(bwt);
        }

        needs_update_ = false;
        vertexs_.clear();
      }
    }

    if (rb_solid_->isChecked()) {
      glColor4f(color_.redF(), color_.greenF(), color_.blueF(), alpha_);
    }
    glPointSize(point_size_ * 10.f / global_data_->camera_->getEyeDistance());

    for (auto &bwt : buffers_) {
      GLPushGuard pg;
      // transform(bwt.frame_id, "global", bwt.utime);

      glDisable(GL_DEPTH_TEST);
      drawArrays(GL_POINTS, bwt.buffer);
      glEnable(GL_DEPTH_TEST);
    }
  }

  void update(const std::shared_ptr<crdc::airi::PointClouds2> &_msg) {
    if (_msg->clouds_size() <= 0) {                         \
      LOG(WARNING) << (_msg->clouds_size() <= 0) << " is not met.";
      return;
    }

    // initialize once
    if (!initialized_) {
      initialized_ = true;
      for (const auto &field : _msg->clouds(0).fields()) {
        cb_render_field_->addItem(QString::fromStdString(field.name()));
      }
    }

    // setup sub-items
    for (const auto &cloud : _msg->clouds()) {
      const auto frame_id = cloud.header().frame_id();
      if (enables_.find(frame_id) == enables_.end()) {
        enables_[frame_id] = true;
        auto cb_enable =
            new CheckBox(QString::fromStdString(frame_id), enables_[frame_id],
                         [&, frame_id](bool is_checked) { enables_[frame_id] = is_checked; });
        item_->addWidget(cb_enable);
      }
    }

    // merge
    const auto &front = _msg->clouds(0);
    auto msg = std::make_shared<crdc::airi::PointCloud2>();
    *(msg->mutable_header()) = _msg->header();
    *(msg->mutable_fields()) = front.fields();
    msg->set_point_step(front.point_step());
    int num_points = 0;
    bool is_dense = true, is_bigendian = true;
    for (const auto &cloud : _msg->clouds()) {
      if (!enables_[cloud.header().frame_id()]) {
        continue;
      }
      num_points += cloud.height() * cloud.width();
      if (!cloud.is_dense()) {
        is_dense = false;
      }
      if (!cloud.is_bigendian()) {
        is_bigendian = false;
      }
    }
    msg->set_height(1);
    msg->set_width(num_points);
    msg->set_row_step(msg->point_step() * num_points);
    msg->set_is_dense(is_dense);
    msg->set_is_bigendian(is_bigendian);
    auto data = msg->mutable_data();
    data->resize(msg->row_step());
    int offset = 0;
    for (const auto &cloud : _msg->clouds()) {
      if (!enables_[cloud.header().frame_id()]) {
        continue;
      }
      const int size = cloud.height() * cloud.row_step();
      memcpy((void *)(data->data() + offset), cloud.data().data(), size);
      offset += size;
    }

    // organize data
    std::unordered_map<std::string, std::vector<double>> fields_data;
    for (uint32_t h = 0; h < msg->height(); ++h) {
      auto data = msg->data().data() + msg->row_step() * h;

      for (uint32_t w = 0; w < msg->width(); ++w) {
        for (auto it_field = msg->fields().begin(); it_field != msg->fields().end(); ++it_field) {
          auto p = data + it_field->offset();
          switch (it_field->datatype()) {
            case crdc::airi::PointField_PointFieldType_INT8:
              fields_data[it_field->name()].push_back(*((int8_t *)p));
              break;
            case crdc::airi::PointField_PointFieldType_INT16:
              fields_data[it_field->name()].push_back(*((int16_t *)p));
              break;
            case crdc::airi::PointField_PointFieldType_INT32:
              fields_data[it_field->name()].push_back(*((int32_t *)p));
              break;
            case crdc::airi::PointField_PointFieldType_UINT8:
              fields_data[it_field->name()].push_back(*((uint8_t *)p));
              break;
            case crdc::airi::PointField_PointFieldType_UINT16:
              fields_data[it_field->name()].push_back(*((uint16_t *)p));
              break;
            case crdc::airi::PointField_PointFieldType_UINT32:
              fields_data[it_field->name()].push_back(*((uint32_t *)p));
              break;
            case crdc::airi::PointField_PointFieldType_FLOAT32:
              fields_data[it_field->name()].push_back(*((float *)p));
              break;
            case crdc::airi::PointField_PointFieldType_FLOAT64:
              fields_data[it_field->name()].push_back(*((double *)p));
              break;
            default:
              LOG(WARNING) << "Unknown pcl field type: " << int(it_field->datatype());
              break;
          }
        }
        data += msg->point_step();
      }
    }

    VertexWithTrans vwt;
    vwt.frame_id = msg->header().frame_id();
    vwt.utime = msg->header().lidar_timestamp();

    if (rb_solid_->isChecked()) {
      vwt.vertex = Eigen::MatrixXf(3, msg->height() * msg->width());
      auto px = fields_data["x"].data();
      auto py = fields_data["y"].data();
      auto pz = fields_data["z"].data();
      for (uint32_t i = 0; i < msg->height() * msg->width(); ++i) {
        vwt.vertex(0, i) = *px++;
        vwt.vertex(1, i) = *py++;
        vwt.vertex(2, i) = *pz++;
      }
    } else {
      vwt.vertex = Eigen::MatrixXf(7, msg->height() * msg->width());
      auto px = fields_data["x"].data();
      auto py = fields_data["y"].data();
      auto pz = fields_data["z"].data();
      for (uint32_t i = 0; i < msg->height() * msg->width(); ++i) {
        vwt.vertex(0, i) = *px++;
        vwt.vertex(1, i) = *py++;
        vwt.vertex(2, i) = *pz++;
      }

      const auto &render_field = cb_render_field_->currentText().toStdString();
      auto it = fields_data.find(render_field);
      if (it != fields_data.end()) {
        cv::Mat colormap(1, msg->height() * msg->width(), CV_64FC1, it->second.data());
        bool manual = false;
        if (!cb_render_range_auto_->isChecked()) {
          bool min_ok, max_ok;
          auto min = le_render_min_->text().toDouble(&min_ok);
          auto max = le_render_max_->text().toDouble(&max_ok);
          if (min_ok && max_ok) {
            cv::Mat min_mat(1, msg->height() * msg->width(), CV_64FC1, min);
            cv::Mat max_mat(1, msg->height() * msg->width(), CV_64FC1, max);
            cv::max(colormap, min_mat, colormap);
            cv::min(colormap, max_mat, colormap);
            manual = true;
          }
        }
        if (!manual) {
          double min, max;
          cv::minMaxLoc(colormap, &min, &max);
          le_render_min_->setText(QString::fromStdString(std::to_string(min)));
          le_render_max_->setText(QString::fromStdString(std::to_string(max)));
        }

        cv::normalize(colormap, colormap, 0, 255, cv::NORM_MINMAX, CV_64FC1);
        colormap.convertTo(colormap, CV_8UC1);
        cv::applyColorMap(colormap, colormap, cb_render_type_->currentData().toInt());
        for (size_t i = 0; i < msg->height() * msg->width(); ++i) {
          auto &color = colormap.at<cv::Vec3b>(i);
          vwt.vertex(3, i) = color[2] / 255.f;
          vwt.vertex(4, i) = color[1] / 255.f;
          vwt.vertex(5, i) = color[0] / 255.f;
          vwt.vertex(6, i) = alpha_;
        }
      }
    }

    std::lock_guard<std::mutex> lock(mutex_);
    vertexs_.push_back(std::move(vwt));
    needs_update_ = true;
  }

 protected:
  const std::string channel_;
  bool needs_update_{false};

  struct VertexWithTrans {
    Eigen::MatrixXf vertex;
    std::string frame_id;
    size_t utime;
  };
  boost::circular_buffer<VertexWithTrans> vertexs_;
  boost::circular_buffer<GLBufferWithTrans> buffers_;
  std::mutex mutex_;

  bool initialized_{false};
  float point_size_{1.f};
  float alpha_{1.f};
  QColor color_{Qt::white};
  QRadioButton *rb_solid_;
  QRadioButton *rb_colormap_;
  ColorButton *cb_color_;
  QComboBox *cb_render_type_;
  QComboBox *cb_render_field_;
  CheckBox *cb_render_range_auto_;
  QLineEdit *le_render_min_;
  QLineEdit *le_render_max_;
  RendererItem *item_;
  std::unordered_map<std::string, bool> enables_;
};

PointCloudsRenderer::PointCloudsRenderer() {
  item_ = new RendererItem(
      "PointClouds", global_data_->config_.pointclouds_renderer_enable(),
      [&](bool is_checked) { global_data_->config_.set_pointclouds_renderer_enable(is_checked); });
  global_data_->renderer_manager_->addWidget(item_);
}

bool PointCloudsRenderer::enabled() const {
  return global_data_->config_.pointclouds_renderer_enable();
}

void PointCloudsRenderer::initialize() {
  Renderer::initialize();

  global_data_->message_hub_->subscribe<crdc::airi::PointClouds2>(
      [&](const std::string &channel,
          const std::shared_ptr<crdc::airi::PointClouds2> &msg) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = channels_.find(channel);
        if (it != channels_.end()) {
          channels_[channel]->update(msg);
        } else {
          to_be_added_[channel] = msg;
        }
      });
}

void PointCloudsRenderer::render() {
  // update widgets
  {
    std::unique_lock<std::mutex> lock(mutex_);
    for (auto it = to_be_added_.begin(); it != to_be_added_.end();) {
      const auto &channel = it->first;
      channels_[channel].reset(new PointCloudsChannel(channel, item_));
      channels_[channel]->initialize();
      channels_[channel]->update(it->second);
      it = to_be_added_.erase(it);
    }
  }

  if (!enabled()) {
    return;
  }

  // render
  for (auto &channel : channels_) {
    if (channel.second->enabled()) {
      channel.second->render();
    }
  }
}

void PointCloudsRenderer::loadConfigPost() {
  item_->setChecked(enabled());

  for (auto &channel : channels_) {
    channel.second->loadConfigPost();
  }
}

}  // namespace airi
}  // namespace crdc
