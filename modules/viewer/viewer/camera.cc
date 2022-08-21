#include "viewer/camera.h"
#include <GL/gl.h>
#include <QMouseEvent>
#include <QWheelEvent>

#ifdef __aarch64__
#include <GL/gl.h>

GLenum glCheckError_(const char *file, int line)
{
    GLenum errorCode;
    while ((errorCode = glGetError()) != GL_NO_ERROR)
    {
        std::string error;
        switch (errorCode)
        {
            case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
            case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
            case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
            case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
            case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
            case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
            case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
       }
        std::cout << error << " | " << file << " (" << line << ")" << std::endl;
    }
    return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)
#endif

namespace crdc {
namespace airi {

Camera::Camera() { reset(); }

void Camera::resizeGL(int w, int h) { viewport_ = glm::dvec4(0, 0, w, h); }

void Camera::paintGL() {
  if (callback_camera_control_) {
    callback_camera_control_();
  }

  update();

#ifdef __aarch64__
#else
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glMultMatrixd(glm::value_ptr(projection_matrix_));

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glMultMatrixd(glm::value_ptr(model_matrix_));
#endif
}

#ifdef __aarch64__
glm::dmat4x4 Camera::getProjectionMatrix() {
    return projection_matrix_;
}

glm::dmat4x4 Camera::getModelMatrix() {
    return model_matrix_;
}
#endif

void Camera::mousePressEvent(QMouseEvent *e) {
  pt_previous_ = e->pos();

  if (e->button() == Qt::LeftButton) {
    is_panning_ = true;
  } else if (e->button() == Qt::RightButton) {
    is_rotating_ = true;
  }
}

void Camera::mouseReleaseEvent(QMouseEvent *e) {
  if (e->button() == Qt::LeftButton) {
    is_panning_ = false;
  } else if (e->button() == Qt::RightButton) {
    is_rotating_ = false;
  }
}

void Camera::mouseMoveEvent(QMouseEvent *e) {
  if (is_panning_) {
    const auto &pt_start = getWorldPoint(glm::dvec3(pt_previous_.x(), pt_previous_.y(), 0),
                                         glm::dvec3(0, 0, lookat_.z));
    const auto &pt_current =
        getWorldPoint(glm::dvec3(e->pos().x(), e->pos().y(), 0), glm::dvec3(0, 0, lookat_.z));
    translate(pt_start - pt_current);
    pt_previous_ = e->pos();
  } else if (is_rotating_) {
    const auto delta = e->pos() - pt_previous_;
    if (lock_birdview_) {
      rotateYaw(delta.x());
    }
    else {
      rotateYawPitch(delta.x(), delta.y());
    }
    pt_previous_ = e->pos();
  }
}

void Camera::wheelEvent(QWheelEvent *e) {
  if (std::fabs(eye_.z) < std::numeric_limits<double>::epsilon()) {
    zoom(e->delta() > 0 ? 0.9 : 1.1);
  } else {
    zoomAndPan(e->delta(), e->pos().x(), e->pos().y());
  }
}

void Camera::lockBirdview(const bool lock) {
  lock_birdview_ = lock;
}

glm::dvec3 Camera::getEye() const { return eye_; }

double Camera::getEyeDistance() const { return eye_distance_; }

double Camera::getLookatZ() const {
  return lookat_.z;
}

void Camera::setLookatZ(const double &z) { lookat_.z = z; }

glm::dvec3 Camera::getScreenPoint(const glm::dvec3 &pt_world) const {
  return glm::project(pt_world, model_matrix_, projection_matrix_, viewport_);
}

glm::dvec3 Camera::getWorldPoint(const glm::dvec3 &pt_screen, const glm::dvec3 &plane_center,
                                 const glm::dvec3 &plane_normal) const {
  const auto ray_end = glm::unProject(glm::dvec3{pt_screen.x, viewport_[3] - pt_screen.y, 0},
                                      model_matrix_, projection_matrix_, viewport_);
  const auto ray_start = glm::unProject(glm::dvec3{pt_screen.x, viewport_[3] - pt_screen.y, 1},
                                        model_matrix_, projection_matrix_, viewport_);
  const auto ray_dir = glm::normalize(ray_end - ray_start);

  const auto denominator = glm::dot(ray_dir, plane_normal);
  if (std::fabs(denominator) < std::numeric_limits<double>::epsilon()) {
    return glm::dvec3(0, 0, 0);
  }

  const double distance = glm::dot(plane_center - ray_start, plane_normal) / denominator;
  if (distance < 0) {
    return glm::dvec3(0, 0, 0);
  }

  return ray_start + distance * ray_dir;
}

void Camera::reset() {
  eye_ = glm::dvec3(0, 0, 50);
  lookat_ = glm::dvec3(0, 0, 0);
  up_ = glm::dvec3(0, 1, 0);
  update();
}

void Camera::jumpTo(const glm::dvec3 &pos, const double heading) {
  double distance_xy = std::hypot(lookat_[0] - eye_[0], lookat_[1] - eye_[1]);
  eye_[0] = pos[0] - distance_xy * std::cos(heading);
  eye_[1] = pos[1] - distance_xy * std::sin(heading);
  up_[0] = eye_[2] * std::cos(heading);
  up_[1] = eye_[2] * std::sin(heading);
  up_[2] = distance_xy;
  lookat_ = pos;
}

void Camera::translate(const glm::dvec3 &trans) {
  eye_ += trans;
  lookat_ += trans;
  update();
}

void Camera::rotateYaw(const int dx) {
  rotateYawPitch(dx, 0);
}

void Camera::rotateYawPitch(const int dx, const int dy) {
  const auto view = lookat_ - eye_;
  const auto left = glm::normalize(glm::cross(up_, view));

  const auto q_yaw = glm::angleAxis(-dx * 0.01, glm::dvec3(0, 0, 1));
  const auto q_pitch = glm::angleAxis(dy * 0.005, left);

  auto new_view = q_yaw * q_pitch * view;

  auto new_eye = lookat_ - new_view;
  auto new_up = glm::normalize(glm::cross(new_view, q_yaw * left));

  if (new_up.z < 0.01) {
    new_eye = lookat_ + glm::dvec3(0, 0, eye_distance_);
    new_view = lookat_ - new_eye;
    new_up.z = 0;
    new_up =
        glm::normalize(glm::cross(new_view, q_yaw * glm::normalize(glm::cross(new_up, new_view))));
    new_up.z = 0;
  }
  // else if (new_eye.z < lookat_.z + 0.1) {
  //   new_eye.z = lookat_.z;
  //   new_view = lookat_ - new_eye;
  //   new_up.x = 0;
  //   new_up.y = 0;
  //   new_eye = lookat_ - q_yaw * glm::angleAxis(dy * 0.005, left) * new_view;
  //   new_eye.z = lookat_.z;
  // }

  eye_ = new_eye;
  up_ = new_up;
  update();
}

void Camera::rotateAndTranslate(const glm::dvec3 &pos_prev, const glm::dvec3 &pos,
                                const glm::dquat &quat_prev, const glm::dquat &quat) {
  const auto mat3x3 = glm::dmat3x3(glm::rotate(
      glm::dmat4x4(1.0), -glm::eulerAngles(quat_prev * glm::inverse(quat)).z, glm::dvec3(0, 0, 1)));
  lookat_ = mat3x3 * (lookat_ - pos_prev) + pos;
  eye_ = mat3x3 * (eye_ - pos_prev) + pos;
  up_ = mat3x3 * up_;
  update();
}

void Camera::zoom(const double ratio) {
  const double eye_distance_new = eye_distance_ * ratio;
  if (eye_distance_new < near_ || eye_distance_new > far_) return;

  eye_ = eye_distance_new * glm::normalize(eye_ - lookat_) + lookat_;
  update();
}

void Camera::zoomAndPan(const int delta, const int x, const int y) {
  const auto pt_start = getWorldPoint(glm::dvec3(x, y, 0), glm::dvec3(0, 0, lookat_.z));
  zoom(delta > 0 ? 0.9 : 1.1);
  const auto &pt_end = getWorldPoint(glm::dvec3(x, y, 0), glm::dvec3(0, 0, lookat_.z));
  translate(pt_start - pt_end);
}

void Camera::setCameraControlCallback(const std::function<void()> &callback_camera_control) {
  callback_camera_control_ = callback_camera_control;
}

void Camera::update() {
  model_matrix_ = glm::lookAt(eye_, lookat_, up_);
  eye_distance_ = glm::length(eye_ - lookat_);

  const auto aspect_ratio = double(viewport_[2]) / double(viewport_[3]);
  if (std::fabs(eye_.z) < std::numeric_limits<double>::epsilon() ||
      std::fabs(up_.z) < std::numeric_limits<double>::epsilon()) {
    const auto c = eye_distance_ / 1.5 * 0.825;
    projection_matrix_ = glm::ortho(-c * aspect_ratio, c * aspect_ratio, -c, c, near_, far_);
  } else {
    projection_matrix_ = glm::perspective(fov_, aspect_ratio, near_, far_);
  }
}

}  // namespace airi
}  // namespace crdc
