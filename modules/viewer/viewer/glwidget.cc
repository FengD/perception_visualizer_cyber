#include "viewer/glwidget.h"
#include <QMouseEvent>
#include <QPainter>
#include <QTimer>
#include <QWheelEvent>
#include "cyber/common/log.h"
#include "viewer/camera.h"
#include "viewer/global_data.h"
#include "viewer/renderers/view_renderer.h"
#include "viewer/renderers/context_renderer.h"
#include "viewer/renderers/frame_renderer.h"
// #include "viewer/renderers/hdmap_renderer.h"
// #include "viewer/renderers/hud_renderer.h"
#include "viewer/renderers/marker_renderer.h"
// #include "viewer/renderers/model_renderer.h"
#include "viewer/renderers/perception_renderer.h"
// #include "viewer/renderers/localization_renderer.h"
// #include "viewer/renderers/routing_renderer.h"
// #include "viewer/renderers/planning_renderer.h"
#include "viewer/renderers/pointcloud_renderer.h"
#include "viewer/renderers/pointclouds_renderer.h"
// #include "viewer/renderers/prediction_renderer.h"
#include "viewer/renderers/texturemap_renderer.h"
#ifdef __aarch64__
#include <QOpenGLShaderProgram>
#include <QCoreApplication>
#include <glm/glm.hpp>
#endif

namespace crdc {
namespace airi {

#ifdef __aarch64__
static const char *vertexShaderSource =
    "attribute vec4 vertex;\n"
    "uniform mat4 projMatrix;\n"
    "uniform mat4 mvMatrix;\n"
    "void main() {\n"
    "   gl_Position = projMatrix * mvMatrix * vertex;\n"
    "}\n";

static const char *fragmentShaderSource =
    "uniform highp vec4 color;\n"
    "void main() {\n"
    "   gl_FragColor = color;\n"
    "}\n";
#endif

GLWidget::GLWidget() {
  global_data_ = Singleton<GlobalData>::get();
  global_data_->camera_.reset(new Camera());
}

#ifdef __aarch64__
void GLWidget::setColor(QVector4D color) {
  //glCheckError();
  m_program->setUniformValue(m_color, color);
  //glCheckError();
}
#endif

void GLWidget::loadConfigPost() {
  for (auto &renderer : renderers_) {
    renderer->loadConfigPost();
  }
}

void GLWidget::initializeGL() {
  const auto &background_color = global_data_->config_.background_color();
  glClearColor(background_color.r(), background_color.g(), background_color.b(),
               background_color.a());

  // order of construction influences order in renderer manager
  auto view_renderer = std::make_shared<ViewRenderer>();
  auto context_renderer = std::make_shared<ContextRenderer>();
  // auto model_renderer = std::make_shared<ModelRenderer>();
  auto frame_renderer = std::make_shared<FrameRenderer>();
  // auto hdmap_renderer = std::make_shared<HDMapRenderer>();
  auto texturemap_renderer = std::make_shared<TexturemapRenderer>();
  auto pointcloud_renderer = std::make_shared<PointCloudRenderer>();
  auto pointclouds_renderer = std::make_shared<PointCloudsRenderer>();
  // auto localization_renderer = std::make_shared<LocalizationRenderer>();
  auto perception_renderer = std::make_shared<PerceptionRenderer>();
  // auto prediction_renderer = std::make_shared<PredictionRenderer>();
  // auto routing_renderer = std::make_shared<RoutingRenderer>();
  // auto planning_renderer = std::make_shared<PlanningRenderer>();
  auto marker_renderer = std::make_shared<MarkerRenderer>();
  // auto hud_renderer = std::make_shared<HudRenderer>();

  // order in renderers_ decides order of rendering
  renderers_.push_back(view_renderer);
  renderers_.push_back(context_renderer);
  // renderers_.push_back(hdmap_renderer);
  renderers_.push_back(texturemap_renderer);
  // renderers_.push_back(localization_renderer);
  // renderers_.push_back(routing_renderer);
  // renderers_.push_back(prediction_renderer);
  // renderers_.push_back(planning_renderer);
  renderers_.push_back(pointcloud_renderer);
  renderers_.push_back(pointclouds_renderer);
  renderers_.push_back(perception_renderer);
  renderers_.push_back(frame_renderer);
  // renderers_.push_back(model_renderer);
  renderers_.push_back(marker_renderer);
  // renderers_.push_back(hud_renderer);

#ifdef __aarch64__
  m_program = new QOpenGLShaderProgram;
  m_program->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderSource);
  m_program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSource);
  m_program->bindAttributeLocation("vertex", 0);
  m_program->link();

  m_program->bind();
  m_projMatrixLoc = m_program->uniformLocation("projMatrix");
  m_mvMatrixLoc = m_program->uniformLocation("mvMatrix");
  m_color = m_program->uniformLocation("color");

  // Setup our vertex buffer object.
  //m_logoVbo.create();
  //m_logoVbo.bind();
  //m_logoVbo.allocate(m_logo.constData(), m_logo.count() * sizeof(GLfloat));


  //m_logoVbo.bind();
  //QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
  //f->glEnableVertexAttribArray(0);
  //f->glEnableVertexAttribArray(1);
  //f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), 0);
  //f->glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), reinterpret_cast<void *>(3 * sizeof(GLfloat)));
  //m_logoVbo.release();

  m_program->setUniformValue(m_color, QVector4D(0.8, 0.8, 0.8, 1.0));

  //m_program->release();
#endif

  for (auto &renderer : renderers_) {
    renderer->initialize();
  }

  auto timer = new QTimer();
  QObject::connect(timer, &QTimer::timeout, [&]() { this->update(); });
  timer->start(33);
}

void GLWidget::resizeGL(int w, int h) {
  glViewport(0, 0, w, h);
  global_data_->camera_->resizeGL(w, h);
}

void GLWidget::paintGL() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_POINT_SMOOTH);
  glEnable(GL_LINE_SMOOTH);

  global_data_->camera_->setLookatZ(global_data_->pose()->pose().position().z());
  global_data_->camera_->paintGL();

#ifdef __aarch64__
  glm::dmat4x4 project_tmp = global_data_->camera_->getProjectionMatrix();
  glm::dmat4x4 model_tmp = global_data_->camera_->getModelMatrix();

  for (int i = 0; i <4;i++) {
    for (int j = 0; j < 4; j++) {
      m_model(i,j) = model_tmp[j][i];
      m_proj(i,j) = project_tmp[j][i];     
    }
  }

  m_program->bind();
  m_program->setUniformValue(m_projMatrixLoc, m_proj);
  m_program->setUniformValue(m_mvMatrixLoc, m_model);
#endif

  for (auto &renderer : renderers_) {
    if (global_data_->config_.has_default_color()) {
      const auto &color = global_data_->config_.default_color();
      glColor4f(color.r(), color.g(), color.b(), color.a());
    }
    if (global_data_->config_.has_default_point_size()) {
      glPointSize(global_data_->config_.default_point_size());
    }
    if (global_data_->config_.has_default_line_width()) {
      glLineWidth(global_data_->config_.default_line_width());
    }

    try {
      GLPushGuard pg;
      renderer->render();
    } catch (std::exception &e) {
      LOG(ERROR) << renderer->name() << ": " << e.what();
    }
  }
#ifdef __aarch64__
  m_program->release();
#endif
}

void GLWidget::mousePressEvent(QMouseEvent *e) {
  global_data_->pt_mouse_press_ = e->pos();
  global_data_->pt_mouse_current_ = e->pos();

  if (global_data_->bt_measure_->isChecked()) {
    global_data_->measuring_ = true;
    const auto &pt3d =
        global_data_->camera_->getWorldPoint({e->pos().x(), e->pos().y(), 0});
    global_data_->pt3d_mouse_press_.setX(pt3d.x);
    global_data_->pt3d_mouse_press_.setY(pt3d.y);
  }
  else {
    const auto &pt3d_press = global_data_->camera_->getWorldPoint(
        glm::dvec3(global_data_->pt_mouse_press_.x(), global_data_->pt_mouse_press_.y(), 0));
    std::stringstream ss;
    ss << std::setiosflags(std::ios::fixed) << std::setprecision(2);
    ss << "(" << pt3d_press.x << ", " << pt3d_press.y << ")";
    global_data_->bt_measure_->setText(QString::fromStdString(ss.str()));

    global_data_->camera_->mousePressEvent(e);

    if (e->modifiers() & Qt::ControlModifier) {
      auto p = global_data_->camera_->getWorldPoint({e->pos().x(), e->pos().y(), 0});
      crdc::airi::PointENU pt;
      pt.set_x(p.x);
      pt.set_y(p.y);
      global_data_->routing_points_.push_back(pt);
    }
  }
}

void GLWidget::mouseReleaseEvent(QMouseEvent *e) {
  if (global_data_->measuring_) {
    global_data_->bt_measure_->setChecked(false);
    global_data_->measuring_ = false;
  }
  else {
    global_data_->camera_->mouseReleaseEvent(e);
  }
}

void GLWidget::mouseMoveEvent(QMouseEvent *e) {
  if (global_data_->measuring_) {
    global_data_->pt_mouse_current_ = e->pos();

    const auto &pt3d_press = global_data_->camera_->getWorldPoint(
        glm::dvec3(global_data_->pt_mouse_press_.x(), global_data_->pt_mouse_press_.y(), 0));
    const auto &pt3d_current = global_data_->camera_->getWorldPoint(
        glm::dvec3(global_data_->pt_mouse_current_.x(), global_data_->pt_mouse_current_.y(), 0));
    std::stringstream ss;
    ss << std::setiosflags(std::ios::fixed) << std::setprecision(2);
    ss << "(" << pt3d_press.x << ", " << pt3d_press.y << ")";
    ss << " to (" << pt3d_current.x << ", " << pt3d_current.y << ")";
    ss << ", L="
        << std::hypot(pt3d_current.x - pt3d_press.x,
                      pt3d_current.y - pt3d_press.y);
    ss << ", θ="
        << std::atan2(pt3d_current.y - pt3d_press.y,
                      pt3d_current.x - pt3d_press.x) * 180.f / M_PI;
    global_data_->bt_measure_->setText(QString::fromStdString(ss.str()));
  }
  else {
    global_data_->camera_->mouseMoveEvent(e);
  }
}

void GLWidget::wheelEvent(QWheelEvent *e) { global_data_->camera_->wheelEvent(e); }

}  // namespace airi
}  // namespace crdc
