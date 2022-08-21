#pragma once

#include <QGLWidget>
#include <list>
#include <memory>
#ifdef __aarch64__
#include <QOpenGLFunctions>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QMatrix4x4>

QT_FORWARD_DECLARE_CLASS(QOpenGLShaderProgram)
#endif

namespace crdc {
namespace airi {

class GlobalData;
class Renderer;

class GLWidget : public QGLWidget {
 public:
  GLWidget();
#ifdef __aarch64__
  void setColor(QVector4D color);
#endif

 public:
  void loadConfigPost();

 protected:
  void initializeGL() override;
  void resizeGL(int w, int h) override;
  void paintGL() override;

 protected:
  void mousePressEvent(QMouseEvent *e) override;
  void mouseReleaseEvent(QMouseEvent *e) override;
  void mouseMoveEvent(QMouseEvent *e) override;
  void wheelEvent(QWheelEvent *e) override;

 protected:
  GlobalData *global_data_;
  std::list<std::shared_ptr<Renderer>> renderers_;
#ifdef __aarch64__
  QOpenGLVertexArrayObject m_vao;
  QOpenGLShaderProgram *m_program;
  int m_projMatrixLoc;
  int m_color;
  int m_mvMatrixLoc;
  int m_normalMatrixLoc;
  int m_lightPosLoc;
  QMatrix4x4 m_proj;
  QMatrix4x4 m_camera;
  QMatrix4x4 m_world;
  QMatrix4x4 m_model;
#endif
};

}  // namespace airi
}  // namespace crdc
