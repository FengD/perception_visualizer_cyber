#pragma once

#include <QPoint>
#include <functional>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

class QMouseEvent;
class QWheelEvent;

namespace crdc {
namespace airi {

class Camera {
 public:
  Camera();

 public:
  void resizeGL(int w, int h);
  void paintGL();

  void mousePressEvent(QMouseEvent *e);
  void mouseReleaseEvent(QMouseEvent *e);
  void mouseMoveEvent(QMouseEvent *e);
  void wheelEvent(QWheelEvent *e);

#ifdef __aarch64__
  glm::dmat4x4 getProjectionMatrix();
  glm::dmat4x4 getModelMatrix();
#endif

  void lockBirdview(const bool lock);

  glm::dvec3 getEye() const;
  double getEyeDistance() const;

  double getLookatZ() const;
  void setLookatZ(const double &z);

  glm::dvec3 getScreenPoint(const glm::dvec3 &pt_world) const;
  glm::dvec3 getWorldPoint(const glm::dvec3 &pt_screen, const glm::dvec3 &plane_center = {0, 0, 0},
                           const glm::dvec3 &plane_normal = {0, 0, 1}) const;

  void reset();
  void jumpTo(const glm::dvec3 &pos, const double heading);
  void translate(const glm::dvec3 &trans);
  void rotateYaw(const int dx);
  void rotateYawPitch(const int dx, const int dy);
  void rotateAndTranslate(const glm::dvec3 &pos_prev, const glm::dvec3 &pos,
                          const glm::dquat &quat_prev, const glm::dquat &quat);
  void zoom(const double ratio);
  void zoomAndPan(const int delta, const int x, const int y);
  void setCameraControlCallback(const std::function<void()> &callback_camera_control);
  void update();

 protected:
  const double fov_{45.0};
  const double near_{0.1};
  const double far_{5000.0};

  glm::dvec3 eye_;
  glm::dvec3 lookat_;
  glm::dvec3 up_;
  glm::ivec4 viewport_;
  double eye_distance_;

  glm::dmat4x4 model_matrix_;
  glm::dmat4x4 projection_matrix_;

  bool lock_birdview_{false};
  bool is_panning_{false};
  bool is_rotating_{false};
  QPoint pt_previous_;

  std::function<void()> callback_camera_control_;
};

}  // namespace airi
}  // namespace crdc
