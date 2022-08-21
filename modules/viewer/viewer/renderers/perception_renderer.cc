#include "viewer/renderers/perception_renderer.h"
#include <QLabel>
#include <QLineEdit>
#include "common/io/file.h"
#include "viewer/global_data.h"
#include "viewer/camera.h"
#include "viewer/renderer_manager.h"
#include "viewer/widgets/check_box.h"
#include "viewer/widgets/push_button.h"
#include "viewer/widgets/renderer_item.h"

namespace crdc {
namespace airi {

static bool near(const double value, const double target) {
  return std::abs(value - target) <= 1e-6;
}

class PerceptionChannel : public Renderer {
 public:
  PerceptionChannel(const std::string &channel, RendererItem *renderer_item) : channel_(channel) {
    item_ = new RendererItem(QString::fromStdString(channel), enabled(), [&](bool is_checked) {
      auto channel_enable = global_data_->config_.mutable_perception_channel_enable();
      (*channel_enable)[channel_] = is_checked;
    });
    renderer_item->addWidget(item_);

    {
      auto label_exp = new QLabel("Input object IDs divided by comma");
      auto hbox = new QHBoxLayout();
      auto label = new QLabel("IDs:");
      auto edit = new QLineEdit();
      auto bt_edit = new PushButton("OK", [&, edit]() { filter_str_ = edit->text(); });
      item_->addWidget(label_exp);
      hbox->addWidget(label);
      hbox->addWidget(edit);
      hbox->addWidget(bt_edit);
      item_->addLayout(hbox);
    }

    auto label_shape = new QLabel("Shape:");
    item_->addWidget(label_shape);

    auto cb_bbox =
        new CheckBox("Bounding Box", show_bbox_, [&](bool is_checked) { show_bbox_ = is_checked; });
    item_->addWidget(cb_bbox);

    auto cb_convex_hull = new CheckBox("Convex Hull", show_convex_hull_,
                                       [&](bool is_checked) { show_convex_hull_ = is_checked; });
    item_->addWidget(cb_convex_hull);

    auto cb_auto = new CheckBox("Auto", false, [&, cb_bbox, cb_convex_hull](bool is_checked) {
      show_auto_ = is_checked;
      cb_bbox->setEnabled(!is_checked);
      cb_convex_hull->setEnabled(!is_checked);
    });
    cb_auto->click();
    item_->addWidget(cb_auto);

    auto label_info = new QLabel("Information:");
    item_->addWidget(label_info);

    auto cb_icon =
        new CheckBox("Icon", show_icon_, [&](bool is_checked) { show_icon_ = is_checked; });
    item_->addWidget(cb_icon);

    auto cb_id = new CheckBox("ID", show_id_, [&](bool is_checked) { show_id_ = is_checked; });
    item_->addWidget(cb_id);

    auto cb_distance = new CheckBox("Distance", show_distance_,
                                    [&](bool is_checked) { show_distance_ = is_checked; });
    item_->addWidget(cb_distance);

    auto cb_velocity = new CheckBox("Velocity", show_velocity_,
                                    [&](bool is_checked) { show_velocity_ = is_checked; });
    item_->addWidget(cb_velocity);

    auto cb_acceleration = new CheckBox("Acceleration", show_acceleration_,
                                        [&](bool is_checked) { show_acceleration_ = is_checked; });
    item_->addWidget(cb_acceleration);

    auto cb_yaw_rate = new CheckBox("Yaw Rate", show_yaw_rate_,
                                    [&](bool is_checked) { show_yaw_rate_ = is_checked; });
    item_->addWidget(cb_yaw_rate);

    auto cb_track_status = new CheckBox("Track Status", show_track_status_,
                                        [&](bool is_checked) { show_track_status_ = is_checked; });
    item_->addWidget(cb_track_status);

    auto cb_light_status = new CheckBox("Light Status", show_light_status_,
                                        [&](bool is_checked) { show_light_status_ = is_checked; });
    item_->addWidget(cb_light_status);

    auto cb_subtype = new CheckBox("Subtype", show_subtype_,
                                   [&](bool is_checked) { show_subtype_ = is_checked; });
    item_->addWidget(cb_subtype);
    auto cb_min_height = new CheckBox("MinHeight", show_cb_min_height_,
                                   [&](bool is_checked) { show_cb_min_height_ = is_checked; });
    item_->addWidget(cb_min_height);

    auto cb_t_init2now = new CheckBox("t_init2now", show_t_init2now_,
                                   [&](bool is_checked) { show_t_init2now_ = is_checked; });
    item_->addWidget(cb_t_init2now);

    auto cb_t_update2now = new CheckBox("t_update2now", show_t_update2now_,
                                      [&](bool is_checked) { show_t_update2now_ = is_checked; });
    item_->addWidget(cb_t_update2now);

    auto cb_height = new CheckBox("height", show_height_,
                                  [&](bool is_checked) { show_height_ = is_checked; });
    item_->addWidget(cb_height);
  }

 public:
  std::string name() const override { return "PerceptionRenderer/" + channel_; }

  bool enabled() const override {
    auto it = global_data_->config_.perception_channel_enable().find(channel_);
    return (it != global_data_->config_.perception_channel_enable().end() && (it->second));
  }

  void render() override {
    if (!msg_) {
      return;
    }

    if (global_data_->config_.has_perception_line_width()) {
      glLineWidth(global_data_->config_.perception_line_width());
    }

    std::set<int32_t> target_ids;
    auto items = filter_str_.split(",");
    for (const auto &item : items) {
      try {
        int id;
        bool ok;
        id = item.toInt(&ok);
        if (ok) {
          target_ids.insert(id);
        }
      } catch (std::exception &e) {
        LOG(WARNING) << "Failed to cast " << item.toStdString() << " to ID";
      }
    }

    GLPushGuard pg;

    bool is_global = !(msg_->header().has_frame_id() && msg_->header().frame_id() != "global");
    Eigen::Affine3f affine2global;
    if (is_global) {
      glTranslatef(0, 0, global_data_->pose()->pose().position().z());

      affine2global = Eigen::Translation3f::Identity() * Eigen::Quaternionf::Identity();
    } else {
      // transform(msg_->header().frame_id(), "global", msg_->header().timestamp_sec() * 1000000);

      // auto trans = global_data_->tf_->lookupTransform("global", msg_->header().frame_id(),
      //                                                 msg_->header().timestamp_sec() * 1000000);
      // Eigen::Translation3f t(trans.transform.translation.x, trans.transform.translation.y,
      //                        trans.transform.translation.z);
      // Eigen::Quaternionf q(trans.transform.rotation.w, trans.transform.rotation.x,
      //                      trans.transform.rotation.y, trans.transform.rotation.z);
      // affine2global = t * q;
    }

    for (const auto &obstacle : msg_->perception_obstacle()) {
      if (!target_ids.empty() && target_ids.find(obstacle.id()) == target_ids.end()) {
        continue;
      }

      // set color
      crdc::airi::viewer::Color color;
      switch (obstacle.type()) {
        case crdc::airi::PerceptionObstacle_Type_VEHICLE:
          color = global_data_->config_.perception_color_vehicle();
          break;
        case crdc::airi::PerceptionObstacle_Type_BICYCLE:
          color = global_data_->config_.perception_color_cyclist();
          break;
        case crdc::airi::PerceptionObstacle_Type_PEDESTRIAN:
          color = global_data_->config_.perception_color_pedestrian();
          break;
        case crdc::airi::PerceptionObstacle_Type_UNKNOWN_MOVABLE:
          color = global_data_->config_.perception_color_unknown_movable();
          break;
        case crdc::airi::PerceptionObstacle_Type_UNKNOWN_UNMOVABLE:
          color = global_data_->config_.perception_color_unknown_unmovable();
          break;
        default:
          global_data_->config_.default_color();
          break;
      }
#ifdef __aarch64__
      global_data_->glwidget_->setColor(QVector4D(color.r(), color.g(), color.b(), color.a()));
#else
      glColor4f(color.r(), color.g(), color.b(), color.a());
#endif
      

      switch (obstacle.sub_type()) {
        case crdc::airi::PerceptionObstacle_SubType_ST_TRAFFICCONE:
        case crdc::airi::PerceptionObstacle_SubType_st_UNKNOWN_UNMOVABLE_TRAFFIC_CONE: {
          auto &color = global_data_->config_.perception_color_traffic_cone();
#ifdef __aarch64__
          global_data_->glwidget_->setColor(QVector4D(color.r(), color.g(), color.b(), color.a()));
#else
          glColor4f(color.r(), color.g(), color.b(), color.a());
#endif
          
          break;
        }
        case crdc::airi::PerceptionObstacle_SubType_st_UNKNOWN_UNMOVABLE_FENCE: {
          auto &color = global_data_->config_.perception_color_fence();
#ifdef __aarch64__
          global_data_->glwidget_->setColor(QVector4D(color.r(), color.g(), color.b(), color.a()));
#else
          glColor4f(color.r(), color.g(), color.b(), color.a());
#endif
          break;
        }
        default:
          break;
      }

      // prepare shape selection
      bool show_bbox;
      bool show_convex_hull;
      if (show_auto_) {
        if (obstacle.shapetype() == crdc::airi::PerceptionObstacle_ShapeType_BBOX) {
          show_bbox = true;
          show_convex_hull = false;
        } else {
          show_bbox = false;
          show_convex_hull = true;
        }
      } else {
        show_bbox = show_bbox_;
        show_convex_hull = show_convex_hull_;
      }

      // draw shape
      if (show_bbox) {
        drawBoundingBox(Eigen::Vector3f(obstacle.position().x(), obstacle.position().y(),
                                        obstacle.height() / 2),
                        Eigen::Vector3f(obstacle.length(), obstacle.width(), obstacle.height()),
                        obstacle.theta());
      }
      if (show_convex_hull) {
        std::vector<Eigen::Vector2f> points;
        for (const auto &pt : obstacle.polygon_point()) {
          points.emplace_back(pt.x(), pt.y());
        }
        if (points.size() >= 3) {
          drawConvexCylinder(points, obstacle.height());
        }
      }

      // id
      if (show_id_) {
        drawText(Eigen::Vector3f(obstacle.position().x(), obstacle.position().y(),
                                 obstacle.height() + .5f),
                 std::to_string(obstacle.id()), 20, true);
      }

      // distance
      if (show_distance_) {
        const auto distance =
            std::hypot(obstacle.position().x() - global_data_->pose()->pose().position().x(),
                       obstacle.position().y() - global_data_->pose()->pose().position().y());
        static char distance_str[128];
        sprintf(distance_str, "%.2f", distance);
        drawText(Eigen::Vector3f(obstacle.position().x(), obstacle.position().y(),
                                 obstacle.height() + .5f),
                 distance_str, 20, true);
      }

      // velocity
      if (show_velocity_) {
        const auto velocity = std::hypot(obstacle.velocity().x(), obstacle.velocity().y());
        static char velocity_str[128];
        sprintf(velocity_str, "%.2f", velocity);
        drawText(Eigen::Vector3f(obstacle.position().x(), obstacle.position().y(),
                                 obstacle.height() + .5f),
                 velocity_str, 20, true);
        drawArrow(
            Eigen::Vector3f(obstacle.position().x(), obstacle.position().y(), obstacle.height()),
            Eigen::Vector3f(obstacle.position().x() + obstacle.velocity().x(),
                            obstacle.position().y() + obstacle.velocity().y(), obstacle.height()));
      }

      // acceleration
      if (show_acceleration_) {
        const auto acceleration =
            std::hypot(obstacle.acceleration().x(), obstacle.acceleration().y());
        static char acceleration_str[128];
        sprintf(acceleration_str, "%.2f", acceleration);
        drawText(Eigen::Vector3f(obstacle.position().x(), obstacle.position().y(),
                                 obstacle.height() + .5f),
                 acceleration_str, 20, true);
        drawArrow(
            Eigen::Vector3f(obstacle.position().x(), obstacle.position().y(), obstacle.height()),
            Eigen::Vector3f(obstacle.position().x() + obstacle.acceleration().x(),
                            obstacle.position().y() + obstacle.acceleration().y(),
                            obstacle.height()));
      }

      // yaw rate
      if (show_yaw_rate_) {
        static char yaw_rate_str[128];
        sprintf(yaw_rate_str, "%.2f", obstacle.yaw_rate());
        drawText(Eigen::Vector3f(obstacle.position().x(), obstacle.position().y(),
                                 obstacle.height() + .5f),
                 yaw_rate_str, 20, true);
      }

      // show t_update2now, update2now
      if(show_t_init2now_) {
        static char t_init2now_str[128];
        sprintf(t_init2now_str, "%.2f", obstacle.tracking_time());
        drawText(Eigen::Vector3f(obstacle.position().x(), obstacle.position().y(),
                                 obstacle.height() + .5f), t_init2now_str, 20, true);
      }

      if(show_t_update2now_) {
        static char t_update2now_str[128];
        sprintf(t_update2now_str, "%.2f", obstacle.track_status_reside_length());
        drawText(Eigen::Vector3f(obstacle.position().x(), obstacle.position().y(),
                                 obstacle.height() + .5f), t_update2now_str, 20, true);
      }

      if(show_height_) {
        static char height_str[128];
        sprintf(height_str, "%.2f", obstacle.height());
        drawText(Eigen::Vector3f(obstacle.position().x(), obstacle.position().y(),
                                 obstacle.height() + .5f), height_str, 20, true);
      }
      // track status
      if (show_track_status_) {
        std::string str;
        switch (obstacle.track_status()) {
          case 0:
            str = "Static";
            break;
          case 1:
            str = "Moving";
            break;
          default:
            str = "Unknown";
            break;
        }
        drawText(Eigen::Vector3f(obstacle.position().x(), obstacle.position().y(),
                                 obstacle.height() + .5f),
                 str, 20, true);
      }

      // light status
      if (show_light_status_) {
        GLPushGuard pg;
        glTranslatef(obstacle.position().x(), obstacle.position().y(), obstacle.position().z());
        glRotatef(obstacle.theta() * 180.f / M_PI, 0, 0, 1);
        glPushAttrib(GL_CURRENT_BIT);

        const auto status = obstacle.light_status();
        if (near(status.brake_visible(), 1.0)) {
          LOG(INFO) << "brake visiable";
#ifdef __aarch64__
          if (near(status.brake_switch_on(), 1.0)) {
            global_data_->glwidget_->setColor(QVector4D(1, 0, 0, .8f));
          } else {
            global_data_->glwidget_->setColor(QVector4D(.8f, .8f, .8f, .8f));
          }
#else
          if (near(status.brake_switch_on(), 1.0)) {
            glColor4f(1, 0, 0, .8f);
          } else {
            glColor4f(.8f, .8f, .8f, .8f);
          }
#endif
          drawSphere(Eigen::Vector3f(-obstacle.length() / 2, 0, 0), 0.1f);
        }
        if (near(status.left_turn_visible(), 1.0)) {
          

#ifdef __aarch64__
          if (near(status.left_turn_switch_on(), 1.0)) {
            global_data_->glwidget_->setColor(QVector4D(1, 1, 0, .8f));
          } else {
            global_data_->glwidget_->setColor(QVector4D(.8f, .8f, .8f, .8f));
          }
#else
          if (near(status.left_turn_switch_on(), 1.0)) {
            glColor4f(1, 1, 0, .8f);
          } else {
            glColor4f(.8f, .8f, .8f, .8f);
          }
#endif
          drawSphere(Eigen::Vector3f(-obstacle.length() / 2, obstacle.width() / 2, 0), 0.1f);
        }
        if (near(status.right_turn_visible(), 1.0)) {
#ifdef __aarch64__
          if (near(status.right_turn_switch_on(), 1.0)) {
            global_data_->glwidget_->setColor(QVector4D(1, 1, 0, .8f));
          } else {
            global_data_->glwidget_->setColor(QVector4D(.8f, .8f, .8f, .8f));
          }
#else
          if (near(status.right_turn_switch_on(), 1.0)) {
            glColor4f(1, 1, 0, .8f);
          } else {
            glColor4f(.8f, .8f, .8f, .8f);
          }
#endif
          drawSphere(Eigen::Vector3f(-obstacle.length() / 2, -obstacle.width() / 2, 0), 0.1f);
        }
        glPopAttrib();
      }

      // subtype
      if (show_subtype_) {
        std::string str;
        switch (obstacle.obstacle_sub_type()) {
          case crdc::airi::PerceptionObstacle_ObstacleSubType_SUB_TYPE_VEHICLE_CAR:
            str = "VEHICLE_CAR";
            break;
          case crdc::airi::PerceptionObstacle_ObstacleSubType_SUB_TYPE_VEHICLE_XG:
            str = "VEHICLE_XG";
            break;
          case crdc::airi::PerceptionObstacle_ObstacleSubType_SUB_TYPE_VEHICLE_EXPRESS:
            str = "VEHICLE_EXPRESS";
            break;
          case crdc::airi::PerceptionObstacle_ObstacleSubType_SUB_TYPE_VEHICLE_OTHER:
            str = "VEHICLE_OTHER";
            break;
          case crdc::airi::PerceptionObstacle_ObstacleSubType_SUB_TYPE_PEDESTRIAN_POLICE:
            str = "PEDESTRIAN_POLICE";
            break;
          case crdc::airi::
              PerceptionObstacle_ObstacleSubType_SUB_TYPE_PEDESTRIAN_CONSTRUCTER:
            str = "PRDESTRIAN_CONSTRUCTER";
            break;
          case crdc::airi::PerceptionObstacle_ObstacleSubType_SUB_TYPE_PEDESTRIAN_CHILD:
            str = "PEDESTRIAN_CHILD";
            break;
          case crdc::airi::PerceptionObstacle_ObstacleSubType_SUB_TYPE_PEDESTRIAN_RIDER:
            str = "PEDESTRIAN_RIDER";
            break;
          case crdc::airi::PerceptionObstacle_ObstacleSubType_SUB_TYPE_PEDESTRIAN_OTHER:
            str = "PEDESTRIAN_OTHER";
            break;
          case crdc::airi::PerceptionObstacle_ObstacleSubType_SUB_TYPE_BICYCLE_MOTORBICYCLE:
            str = "BICYCLE_MOTORBICYCLE";
            break;
          case crdc::airi::PerceptionObstacle_ObstacleSubType_SUB_TYPE_BICYCLE_MOTORTRICYCLE:
            str = "BICYCLE_MOTORTRICYCLE";
            break;
          case crdc::airi::PerceptionObstacle_ObstacleSubType_SUB_TYPE_BICYCLE_DELIVERYMAN:
            str = "BICYCLE_DELIVERYMAN";
            break;
          case crdc::airi::PerceptionObstacle_ObstacleSubType_SUB_TYPE_TRUCK_BUS:
            str = "TRUCK_BUS";
            break;
          case crdc::airi::
              PerceptionObstacle_ObstacleSubType_SUB_TYPE_TRUCK_CONSTRUCTION_VEHICLE:
            str = "TRUCK_CONSTRUCTION_VEHICLE";
            break;
          case crdc::airi::PerceptionObstacle_ObstacleSubType_SUB_TYPE_TRUCK_PICKUP:
            str = "TRUCK_PICKUP";
            break;
          case crdc::airi::PerceptionObstacle_ObstacleSubType_SUB_TYPE_TRUCK_TRAILER_HEAD:
            str = "TRUCK_TRAILER_HEAD";
            break;
          case crdc::airi::
              PerceptionObstacle_ObstacleSubType_SUB_TYPE_TRUCK_TRAILER_CONTAINER:
            str = "TRUCK_TRAILER_CONTAINER";
            break;
          case crdc::airi::PerceptionObstacle_ObstacleSubType_SUB_TYPE_TRUCK_SPRINKLER:
            str = "TRUCK_SPRINKLER";
            break;
          case crdc::airi::PerceptionObstacle_ObstacleSubType_SUB_TYPE_TRUCK_OTHER:
            str = "TRUCK_OTHER";
            break;
          case crdc::airi::
              PerceptionObstacle_ObstacleSubType_SUB_TYPE_UNKNOWN_MOVABLE_ANIMAL:
            str = "UNKNOWN_MOVABLE_ANIMAL";
            break;
          case crdc::airi::PerceptionObstacle_ObstacleSubType_SUB_TYPE_UNKNOWN_MOVABLE_OTHER:
            str = "UNKNOWN_MOVABLE_OTHER";
            break;
          case crdc::airi::PerceptionObstacle_ObstacleSubType_SUB_TYPE_UNKNOWN_UNMOVABLE_SKY:
            str = "UNKNOWN_UNMOVABLE_SKY";
            break;
          case crdc::airi::
              PerceptionObstacle_ObstacleSubType_SUB_TYPE_UNKNOWN_UNMOVABLE_BUILDING:
            str = "UNKNOWN_UNMOVABLE_BUILDING";
            break;
          case crdc::airi::
              PerceptionObstacle_ObstacleSubType_SUB_TYPE_UNKNOWN_UNMOVABLE_VEGETATION:
            str = "UNKNOWN_UNMOVABLE_VEGETATION";
            break;
          case crdc::airi::
              PerceptionObstacle_ObstacleSubType_SUB_TYPE_UNKNOWN_UNMOVABLE_STONE_BLOCK:
            str = "UNKNOWN_UNMOVABLE_STONE_BLOCK";
            break;
          case crdc::airi::
              PerceptionObstacle_ObstacleSubType_SUB_TYPE_UNKNOWN_UNMOVABLE_PILE:
            str = "UNKNOWN_UNMOVABLE_PILE";
            break;
          case crdc::airi::
              PerceptionObstacle_ObstacleSubType_SUB_TYPE_UNKNOWN_UNMOVABLE_CURB:
            str = "UNKNOWN_UNMOVABLE_CURB";
            break;
          case crdc::airi::
              PerceptionObstacle_ObstacleSubType_SUB_TYPE_UNKNOWN_UNMOVABLE_SPEED_BUMP:
            str = "UNKNOWN_UNMOVABLE_SPEED_BUMP";
            break;
          case crdc::airi::
              PerceptionObstacle_ObstacleSubType_SUB_TYPE_UNKNOWN_UNMOVABLE_TRAFFIC_CONE:
            str = "UNKNOWN_UNMOVABLE_TRAFFIC_CONE";
            break;
          case crdc::airi::
              PerceptionObstacle_ObstacleSubType_SUB_TYPE_UNKNOWN_UNMOVABLE_TRAFFIC_LIGHT:
            str = "UNKNOWN_UNMOVABLE_TRAFFIC_LIGHT";
            break;
          case crdc::airi::
              PerceptionObstacle_ObstacleSubType_SUB_TYPE_UNKNOWN_UNMOVABLE_TRAFFIC_SIGN:
            str = "UNKNOWN_UNMOVABLE_TRAFFIC_SIGN";
            break;
          case crdc::airi::
              PerceptionObstacle_ObstacleSubType_SUB_TYPE_UNKNOWN_UNMOVABLE_WATER_SAFETY_BARRIER:
            str = "UNKNOWN_UNMOVABLE_WATER_SAFETY_BARRIER";
            break;
          case crdc::airi::
              PerceptionObstacle_ObstacleSubType_SUB_TYPE_UNKNOWN_UNMOVABLE_TERRIAN:
            str = "UNKNOWN_UNMOVABLE_TERRIAN";
            break;
          case crdc::airi::
              PerceptionObstacle_ObstacleSubType_SUB_TYPE_UNKNOWN_UNMOVABLE_TREE:
            str = "UNKNOWN_UNMOVABLE_TREE";
            break;
          case crdc::airi::
              PerceptionObstacle_ObstacleSubType_SUB_TYPE_UNKNOWN_UNMOVABLE_FENCE:
            str = "UNKNOWN_UNMOVABLE_FENCE";
            break;
          case crdc::airi::
              PerceptionObstacle_ObstacleSubType_SUB_TYPE_UNKNOWN_UNMOVABLE_POLE:
            str = "UNKNOWN_UNMOVABLE_POLE";
            break;
          case crdc::airi::
              PerceptionObstacle_ObstacleSubType_SUB_TYPE_UNKNOWN_UNMOVABLE_FLOWER_BOX:
            str = "UNKNOWN_UNMOVABLE_FLOWER_BOX";
            break;
          case crdc::airi::
              PerceptionObstacle_ObstacleSubType_SUB_TYPE_UNKNOWN_UNMOVABLE_WARNING_SIGN:
            str = "UNKNOWN_UNMOVABLE_WARNING_SIGN";
            break;
          case crdc::airi::
            PerceptionObstacle_ObstacleSubType_SUB_TYPE_UNKNOWN_UNMOVABLE_CHAIN:
            str = "UNKNOWN_UNMOVABLE_CHAIN";
            break;
          case crdc::airi::
            PerceptionObstacle_ObstacleSubType_SUB_TYPE_GROUND_RUGGED_ROAD:
            str = "GROUND_RUGGED_ROAD";
            break;
          case crdc::airi::
            PerceptionObstacle_ObstacleSubType_SUB_TYPE_UNKNOWN_UNMOVABLE_POTHOLE:
            str = "UNKNOWN_UNMOVABLE_POTHOLE";
            break;
          case crdc::airi::
            PerceptionObstacle_ObstacleSubType_SUB_TYPE_UNKNOWN_UNMOVABLE_VIRTUAL:
            str = "UNKNOWN_UNMOVABLE_VIRTUAL";
            break;
          case crdc::airi::
            PerceptionObstacle_ObstacleSubType_SUB_TYPE_UNKNOWN_UNMOVABLE_OTHER_OBSTACLE:
            str = "SUB_TYPE_UNKNOWN_UNMOVABLE_OTHER_OBSTACLE";
            break;
          case crdc::airi::
            PerceptionObstacle_ObstacleSubType_SUB_TYPE_UNKNOWN_UNMOVABLE_RAIN_DIRT:
            str = "RAID_DIRT";
            break;
          case crdc::airi::
            PerceptionObstacle_ObstacleSubType_SUB_TYPE_UNKNOWN_UNMOVABLE_WATER_PIPE:
            str = "WATER_PIPE";
            break;
          case crdc::airi::
            PerceptionObstacle_ObstacleSubType_SUB_TYPE_UNKNOWN:
            str = "UNKNOWN";
            break;
          default:
            str = "UNDEFINED " + std::to_string(static_cast<int>(obstacle.obstacle_sub_type()));
            break;
        }

        drawText(Eigen::Vector3f(obstacle.position().x(), obstacle.position().y(),
                                 obstacle.height() + .5f),
                 str, 20, true);
      }

      if(show_cb_min_height_) {
        static char min_str[128];
        sprintf(min_str, "%.2f", obstacle.min_height_to_ground());
        drawText(Eigen::Vector3f(obstacle.position().x(), obstacle.position().y(),
                                 obstacle.height() + .5f),
                 min_str, 20, true);
      }

      if (show_icon_) {
        std::string key;
        if (obstacle.type() == crdc::airi::PerceptionObstacle_Type_VEHICLE) {
          if (obstacle.obstacle_sub_type() ==
              crdc::airi::PerceptionObstacle_ObstacleSubType_SUB_TYPE_VEHICLE_XG) {
            key = "xg";
          } else {
            key = "vehicle";
          }
        } else if (obstacle.type() == crdc::airi::PerceptionObstacle_Type_BICYCLE) {
          if (obstacle.obstacle_sub_type() ==
              crdc::airi::PerceptionObstacle_ObstacleSubType_SUB_TYPE_BICYCLE_MOTORBICYCLE) {
            key = "motor_cycle";
          } else {
            key = "bicycle";
          }
        } else if (obstacle.type() == crdc::airi::PerceptionObstacle_Type_PEDESTRIAN) {
          if (obstacle.obstacle_sub_type() ==
              crdc::airi::PerceptionObstacle_ObstacleSubType_SUB_TYPE_PEDESTRIAN_CHILD) {
            key = "child";
          } else {
            key = "pedestrian";
          }
        } else if (obstacle.type() == crdc::airi::PerceptionObstacle_Type_UNKNOWN_MOVABLE) {
          key = "unknown_movable";
        } else if (obstacle.type() ==
                   crdc::airi::PerceptionObstacle_Type_UNKNOWN_UNMOVABLE) {
          if (obstacle.obstacle_sub_type() ==
              crdc::airi::
                  PerceptionObstacle_ObstacleSubType_SUB_TYPE_UNKNOWN_UNMOVABLE_TRAFFIC_CONE) {
            key = "traffic_cone";
          } else if (obstacle.obstacle_sub_type() ==
                     crdc::airi::
                         PerceptionObstacle_ObstacleSubType_SUB_TYPE_UNKNOWN_UNMOVABLE_FENCE) {
            key = "fence";
          } else {
            key = "unknown_unmovable";
          }
        }

        const auto &eye_dis = global_data_->camera_->getEyeDistance();
        int icon_size = eye_dis * (-0.8) + 48;
        icon_size = (icon_size < 12 ? 12 : (icon_size > 48 ? 48 : icon_size));

        auto it = global_data_->textures_.find(key);
        if (it != global_data_->textures_.end()) {
#ifdef __aarch64__
          global_data_->glwidget_->setColor(QVector4D(1.0f, 1.0f, 1.0f, 1.0f));
#else
          glColor4f(1, 1, 1, 1);
#endif
          if (is_global) {
            renderTextureViewFacing(it->second, {
              static_cast<float>(obstacle.position().x()),
              static_cast<float>(obstacle.position().y()),
              static_cast<float>(global_data_->pose()->pose().position().z() +
              obstacle.height()) + 0.35f}, icon_size);
          } else {
            Eigen::Vector3f p(obstacle.position().x(), obstacle.position().y(),
                              obstacle.position().z() + obstacle.height() + 0.35f);
            auto p_glb = affine2global * p;
            renderTextureViewFacing(it->second, {static_cast<float>(p_glb.x()), 
              static_cast<float>(p_glb.y()), static_cast<float>(p_glb.z())}, icon_size);
          }
        }
      }
    }
  }

  void loadConfigPost() override { item_->setChecked(enabled()); }

  void update(const std::shared_ptr<PerceptionObstacles> &msg) { msg_ = msg; }

 protected:
  RendererItem *item_;
  const std::string channel_;
  std::shared_ptr<PerceptionObstacles> msg_;

  QString filter_str_;
  bool show_auto_{false};
  bool show_bbox_{true};
  bool show_convex_hull_{false};
  bool show_icon_{true};
  bool show_id_{false};
  bool show_distance_{false};
  bool show_velocity_{false};
  bool show_acceleration_{false};
  bool show_yaw_rate_{false};
  bool show_track_status_{false};
  bool show_light_status_{false};
  bool show_subtype_{false};
  bool show_cb_min_height_{false};
  bool show_t_init2now_{false};
  bool show_t_update2now_{false};
  bool show_height_{false};
};

PerceptionRenderer::PerceptionRenderer() {
  item_ = new RendererItem(
      "Perception", global_data_->config_.perception_renderer_enable(),
      [&](bool is_checked) { global_data_->config_.set_perception_renderer_enable(is_checked); });
  global_data_->renderer_manager_->addWidget(item_);
}

bool PerceptionRenderer::enabled() const {
  return global_data_->config_.perception_renderer_enable();
}

void PerceptionRenderer::initialize() {
  Renderer::initialize();

  // load textures
  const auto icon_base_dir =
      crdc::airi::util::get_absolute_path(std::getenv("CRDC_WS"), "icons/obstacles");
  auto icon_names = crdc::airi::util::list_sub_paths(icon_base_dir, DT_REG);
  for (const auto &icon_name : icon_names) {
    auto texture_desc =
        loadTextureDesc(crdc::airi::util::get_absolute_path(icon_base_dir, icon_name));
    auto pos = icon_name.rfind('.');
    global_data_->textures_[icon_name.substr(0, pos)] = generateTexture(texture_desc)[0];
  }

  // subscribe message
  global_data_->message_hub_->subscribe<PerceptionObstacles>(
      [&](const std::string &channel, const std::shared_ptr<PerceptionObstacles> &msg) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (channels_.find(channel) == channels_.end()) {
          to_be_added_.insert(channel);
        }
        msgs_[channel] = msg;
        needs_update_[channel] = true;
      });
}

void PerceptionRenderer::render() {
  std::unique_lock<std::mutex> lock(mutex_);

  // glColor4f(1, 1, 1, 1);
  // renderTextureViewFacing(global_data_->textures_["kid"], {5, 10, 3}, 35);

  // update widgets
  for (auto it = to_be_added_.begin(); it != to_be_added_.end();) {
    channels_[*it].reset(new PerceptionChannel(*it, item_));
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

void PerceptionRenderer::loadConfigPost() {
  item_->setChecked(enabled());

  for (auto &channel : channels_) {
    channel.second->loadConfigPost();
  }
}

}  // namespace airi
}  // namespace crdc
