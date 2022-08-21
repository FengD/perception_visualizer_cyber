#include "viewer/renderers/renderer.h"
#include <FTGL/ftgl.h>
#include <GL/freeglut.h>
#include <GL/glut.h>
#include "cyber/common/log.h"
#include "viewer/global_data.h"
#include "viewer/camera.h"
#include "viewer/renderers/ear_cut.h"

namespace crdc {
namespace airi {

Renderer::Renderer() { global_data_ = Singleton<GlobalData>::get(); }

#ifdef __aarch64__
void copy(Eigen::MatrixXf &a, Eigen::MatrixXf &b, int i, int j) {
    a(0, i) = b(0, j);
    a(1, i) = b(1, j);
    a(2, i) = b(2, j);
}
#endif

void Renderer::setColor(const viewer::Color &color) {
  glColor4f(color.r(), color.g(), color.b(), color.a());
}

// void Renderer::transform(const std::string &target_frame_id, const std::string &source_frame_id, const uint64_t utime) {

//   if (!global_data_->tf_->canTransform(source_frame_id, target_frame_id, utime)) {
//     LOG(WARNING) << (!global_data_->tf_->canTransform(source_frame_id, target_frame_id, utime)) << " is not met.";
//     return;
//   }
//   auto trans = global_data_->tf_->lookupTransform(source_frame_id, target_frame_id, utime);

//   const auto &translation = trans.transform.translation;
//   glTranslatef(translation.x, translation.y, translation.z);

//   const auto &rot = trans.transform.rotation;
//   Eigen::Quaterniond quat(rot.w, rot.x, rot.y, rot.z);
//   auto eulers = quat.toRotationMatrix().eulerAngles(1, 0, 2);
//   glRotatef(eulers[1] / M_PI * 180, 0, 1, 0);
//   glRotatef(eulers[0] / M_PI * 180, 1, 0, 0);
//   glRotatef(eulers[2] / M_PI * 180, 0, 0, 1);
// }

void Renderer::drawPoints(const std::vector<Eigen::VectorXf> &points,
                          const std::vector<Eigen::VectorXf> &colors) {
  drawArrays(GL_POINTS, points, colors);
}

void Renderer::drawLines(const std::vector<Eigen::VectorXf> &points,
                         const std::vector<Eigen::VectorXf> &colors) {
  drawArrays(GL_LINES, points, colors);
}

void Renderer::drawLineStrip(const std::vector<Eigen::VectorXf> &points,
                             const std::vector<Eigen::VectorXf> &colors) {
  drawArrays(GL_LINE_STRIP, points, colors);
}

void Renderer::drawLineLoop(const std::vector<Eigen::VectorXf> &points,
                            const std::vector<Eigen::VectorXf> &colors) {
  drawArrays(GL_LINE_LOOP, points, colors);
}

void Renderer::drawArrow(const Eigen::Vector3f &from, const Eigen::Vector3f &to,
                         const float scale) {
  // translate to tail
  GLPushGuard pg;
  glTranslatef(to.x(), to.y(), to.z());
  auto yaw = std::atan2(from.y() - to.y(), from.x() - to.x());
  glRotatef(yaw / M_PI * 180.f, 0, 0, 1);
  auto roll = std::atan2(from.z() - to.z(), std::hypot(from.x() - to.x(), from.y() - to.y()));
  glRotatef(-roll / M_PI * 180.f, 0, 1, 0);

  const float length = std::sqrt(std::pow(from.x() - to.x(), 2) + std::pow(from.y() - to.y(), 2) +
                                 std::pow(from.z() - to.z(), 2));

  // draw body
  glBegin(GL_LINES);
  glVertex3f(0, 0, 0);
  glVertex3f(length, 0, 0);
  glEnd();

  // draw wings
  {
    GLPushGuard pg;
    glRotatef(45.f, 0, 0, 1);
    glBegin(GL_LINES);
    glVertex3f(0, 0, 0);
    glVertex3f(length * scale, 0, 0);
    glEnd();
  }
  {
    GLPushGuard pg;
    glRotatef(-45.f, 0, 0, 1);
    glBegin(GL_LINES);
    glVertex3f(0, 0, 0);
    glVertex3f(length * scale, 0, 0);
    glEnd();
  }
}

void Renderer::drawTriangles(const std::vector<Eigen::VectorXf> &points,
                             const std::vector<Eigen::VectorXf> &colors) {
  drawArrays(GL_TRIANGLES, points, colors);
}

void Renderer::drawEllipse(const Eigen::Vector2f &center, const float radius_x,
                           const float radius_y, const float heading, const bool fill,
                           const float step) {
  GLPushGuard pg;
  glRotatef(heading * 180.f / M_PI, 0, 0, 1);
  if (fill) {
    std::vector<Eigen::Vector2f> points;
    points.push_back(center);
    for (float rad = 0; rad < M_PI * 2; rad += step) {
      points.emplace_back(center.x() + radius_x * std::cos(rad),
                          center.y() + radius_y * std::sin(rad));
    }
    auto vertex = generateVertex(points);

#ifdef __aarch64__
    auto indice = triangulate(points);
    auto buffer = generateGLBuffer(vertex, 2, 0, indice);
#else
    auto buffer = generateGLBuffer(vertex, 2, 0);
#endif
    drawArrays(GL_TRIANGLE_FAN, buffer);
  } else {
    std::vector<Eigen::Vector2f> points;
    for (float rad = 0; rad < M_PI * 2; rad += step) {
      points.emplace_back(center.x() + radius_x * std::cos(rad),
                          center.y() + radius_y * std::sin(rad));
    }
    auto vertex = generateVertex(points);
#ifdef __aarch64__
    auto indice = triangulate(points);
    auto buffer = generateGLBuffer(vertex, 2, 0, indice);
#else
    auto buffer = generateGLBuffer(vertex, 2, 0);
#endif
    drawArrays(GL_LINE_LOOP, buffer);
  }
}

void Renderer::drawCircle(const Eigen::Vector2f &center, const float radius, const bool fill,
                          const float step) {
  drawEllipse(center, radius, radius, 0, fill, step);
}

void Renderer::drawArch(const Eigen::Vector2f &center, const float radius, const float beg,
                        const float end, const bool fill, const float step) {
  if (fill) {
    std::vector<Eigen::Vector2f> points;
    points.push_back(center);
    for (float rad = beg; rad < end; rad += step) {
      points.emplace_back(center.x() + radius * std::cos(rad), center.y() + radius * std::sin(rad));
    }

    auto vertex = generateVertex(points);

#ifdef __aarch64__
    auto indice = triangulate(points);
    auto buffer = generateGLBuffer(vertex, 2, 0, indice);
#else
    auto buffer = generateGLBuffer(vertex, 2, 0);
#endif

    drawArrays(GL_TRIANGLE_FAN, buffer);
  } else {
    std::vector<Eigen::Vector2f> points;
    for (float rad = beg; rad < end; rad += step) {
      points.emplace_back(center.x() + radius * std::cos(rad), center.y() + radius * std::sin(rad));
    }

    auto vertex = generateVertex(points);

#ifdef __aarch64__
    auto indice = triangulate(points);
    auto buffer = generateGLBuffer(vertex, 2, 0, indice);
#else
    auto buffer = generateGLBuffer(vertex, 2, 0);
#endif
    drawArrays(GL_LINE_STRIP, buffer);
  }
}

void Renderer::drawRect(const Eigen::Vector2f &left_top, const Eigen::Vector2f &right_bottom) {
  glBegin(GL_QUADS);
  glVertex2f(left_top.x(), left_top.y());
  glVertex2f(right_bottom.x(), left_top.y());
  glVertex2f(right_bottom.x(), right_bottom.y());
  glVertex2f(left_top.x(), right_bottom.y());
  glEnd();
}

#ifdef __aarch64__
void Renderer::drawRect(const Eigen::Vector4f &xywh) {
    Eigen::MatrixXf point(2, 6);
    point(0, 0) = xywh(0);
    point(1, 0) = xywh(1);

    point(0, 1) = xywh(0) + xywh(2);
    point(1, 1) = xywh(1);

    point(0, 2) = xywh(0) + xywh(2);
    point(1, 2) = xywh(1) + xywh(3);

    point(0, 3) = xywh(0);
    point(1, 3) = xywh(1);

    point(0, 4) = xywh(0) + xywh(2);
    point(1, 4) = xywh(1) + xywh(3);

    point(0, 5) = xywh(0);
    point(1, 5) = xywh(1) + xywh(3);

    auto buffer = generateGLBuffer(point, 2, 0);
    drawArrays(GL_TRIANGLES, buffer);
}

void Renderer::drawRect(const Eigen::MatrixXf& point) {
    //Eigen::MatrixXf point(3, 6);
    //point(0, 0) = xywh(0);
    //point(1, 0) = xywh(1);
    //point(2, 0) = z;

    //point(0, 1) = xywh(0) + xywh(2);
    //point(1, 1) = xywh(1);
    //point(2, 1) = z;

    //point(0, 2) = xywh(0) + xywh(2);
    //point(1, 2) = xywh(1) + xywh(3);
    //point(2, 2) = z;

    //point(0, 3) = xywh(0);
    //point(1, 3) = xywh(1);
    //point(2, 3) = z;

    //point(0, 4) = xywh(0) + xywh(2);
    //point(1, 4) = xywh(1) + xywh(3);
    //point(2, 4) = z;

    //point(0, 5) = xywh(0);
    //point(1, 5) = xywh(1) + xywh(3);
    //point(2, 5) = z;

    auto buffer = generateGLBuffer(point, 2, 0);
    drawArrays(GL_TRIANGLES, buffer);
}
#else
void Renderer::drawRect(const Eigen::Vector4f &xywh) {
  glBegin(GL_QUADS);
  glVertex2f(xywh(0), xywh(1));
  glVertex2f(xywh(0) + xywh(2), xywh(1));
  glVertex2f(xywh(0) + xywh(2), xywh(1) + xywh(3));
  glVertex2f(xywh(0), xywh(1) + xywh(3));
  glEnd();
}
#endif

void Renderer::drawPolygon(const std::vector<Eigen::Vector2f> &polygon) {
  if (polygon.size() < 3) {
    return;
  }
  auto indices = triangulate(polygon);
  auto vertex = generateVertex(polygon);
  auto buffer = generateGLBuffer(vertex, 2, 0, indices);
  drawElements(GL_TRIANGLES, buffer);
}


#ifdef __aarch64__
void Renderer::drawLineAsQuad(const Eigen::Vector2f &start, const Eigen::Vector2f &end, const float width) {
    const auto half_width = width/2;
    const auto angle = std::atan2(end[1] - start[1], end[0] - start[0]);
    const auto sin_angle = half_width*std::sin(angle);
    const auto cos_angle = half_width*std::cos(angle);
    Eigen::MatrixXf point(2, 4);
    point(0, 0) = start[0] - sin_angle;
    point(1, 0) = start[1] + cos_angle;
    point(0, 1) = start[0] + sin_angle;
    point(1, 1) = start[1] - cos_angle;
    point(0, 2) = end[0] + sin_angle;
    point(1, 2) = end[1] - cos_angle;
    point(0, 3) = end[0] - sin_angle;
    point(1, 3) = end[1] + cos_angle;

    Eigen::MatrixXf vertex(3, 6);
    copy(vertex, point, 0, 0);
    copy(vertex, point, 1, 1);
    copy(vertex, point, 2, 2);
    copy(vertex, point, 3, 0);
    copy(vertex, point, 4, 2);
    copy(vertex, point, 5, 3);

    auto buffer = generateGLBuffer(vertex, 3, 0);
    drawArrays(GL_TRIANGLES, buffer);
}
#else
void Renderer::drawLineAsQuad(const Eigen::Vector2f &start, const Eigen::Vector2f &end,
                              const float width) {
  const auto half_width = width / 2;
  const auto angle = std::atan2(end[1] - start[1], end[0] - start[0]);
  const auto sin_angle = half_width * std::sin(angle);
  const auto cos_angle = half_width * std::cos(angle);
  glBegin(GL_QUADS);
  glVertex2d(start[0] - sin_angle, start[1] + cos_angle);
  glVertex2d(start[0] + sin_angle, start[1] - cos_angle);
  glVertex2d(end[0] + sin_angle, end[1] - cos_angle);
  glVertex2d(end[0] - sin_angle, end[1] + cos_angle);
  glEnd();
}
#endif

void Renderer::drawLineStripPolygon(const std::vector<Eigen::Vector2f> &points, const float width) {
  auto polygon = generateLineStripPolygon(points, width);
  drawPolygon(polygon);
}

#ifdef __aarch64__
void Renderer::drawLineStripQuads(const std::vector<Eigen::Vector2f> &points, const float width) {
    //auto points_quads = generateLineStripQuads(points, width);
    //auto vertex = generateVertex(points_quads);
    //auto indice = triangulate(points_quads);
    //auto buffer = generateGLBuffer(vertex, 2, 0, indice);
    //drawArrays(GL_TRIANGLES, buffer);
    drawLineStripPolygon(points, width);

}
#else
void Renderer::drawLineStripQuads(const std::vector<Eigen::Vector2f> &points, const float width) {
  if (points.size() < 2) {
    return;
  }
  auto points_quads = generateLineStripQuads(points, width);
  auto vertex = generateVertex(points_quads);
  auto buffer = generateGLBuffer(vertex, 2, 0);
  drawArrays(GL_QUAD_STRIP, buffer);
}
#endif

void Renderer::drawSphere(const Eigen::Vector3f &center, const float radius) {
  GLPushGuard pg;
  glTranslatef(center.x(), center.y(), center.z());
  glutSolidSphere(radius, radius * 128, radius * 128);
}


#ifdef __aarch64__
void Renderer::drawBoundingBox(const Eigen::Vector3f &center, 
                               const Eigen::Vector3f &lwh, 
                               const float heading, 
                               const bool fill, 
                               const bool show_heading) {
    GLPushGuard pg;

    //glwidget_->camera_->update();
    //glm::dmat4x4 project_tmp = glwidget_->camera_->getProjectionMatrix();
    //glm::dmat4x4 model_tmp = glwidget_->camera_->getModelMatrix();


    //glTranslatef(center.x(), center.y(), center.z());
    //glRotatef(heading/M_PI*180.f, 0, 0, 1);
    //glScalef(lwh.x(), lwh.y(), lwh.z());

    Eigen::MatrixXf point(3, 8);
    
    point(0, 0) = center.x()-lwh.x()/2;
    point(1, 0) = center.y()+lwh.y()/2;
    point(2, 0) = center.z()+lwh.z()/2;

    point(0, 1) = center.x()+lwh.x()/2;
    point(1, 1) = center.y()+lwh.y()/2;
    point(2, 1) = center.z()+lwh.z()/2;

    point(0, 2) = center.x()+lwh.x()/2;
    point(1, 2) = center.y()-lwh.y()/2;
    point(2, 2) = center.z()+lwh.z()/2;

    point(0, 3) = center.x()-lwh.x()/2;
    point(1, 3) = center.y()-lwh.y()/2;
    point(2, 3) = center.z()+lwh.z()/2;

    point(0, 4) = center.x()+lwh.x()/2;
    point(1, 4) = center.y()-lwh.y()/2;
    point(2, 4) = center.z()-lwh.z()/2;

    point(0, 5) = center.x()+lwh.x()/2;
    point(1, 5) = center.y()+lwh.y()/2;
    point(2, 5) = center.z()-lwh.z()/2;

    point(0, 6) = center.x()-lwh.x()/2;
    point(1, 6) = center.y()+lwh.y()/2;
    point(2, 6) = center.z()-lwh.z()/2;

    point(0, 7) = center.x()-lwh.x()/2;
    point(1, 7) = center.y()-lwh.y()/2;
    point(2, 7) = center.z()-lwh.z()/2;

    Eigen::MatrixXf vertex(3, 36);

    copy(vertex, point, 0, 1);
    copy(vertex, point, 1, 0);
    copy(vertex, point, 2, 3);
    copy(vertex, point, 3, 1);
    copy(vertex, point, 4, 3);
    copy(vertex, point, 5, 2);

    copy(vertex, point, 6, 1);
    copy(vertex, point, 7, 2);
    copy(vertex, point, 8, 4);
    copy(vertex, point, 9, 1);
    copy(vertex, point, 10, 4);
    copy(vertex, point, 11, 5);

    copy(vertex, point, 12, 1);
    copy(vertex, point, 13, 5);
    copy(vertex, point, 14, 6);
    copy(vertex, point, 15, 1);
    copy(vertex, point, 16, 6);
    copy(vertex, point, 17, 0);

    copy(vertex, point, 18, 7);
    copy(vertex, point, 19, 6);
    copy(vertex, point, 20, 5);
    copy(vertex, point, 21, 7);
    copy(vertex, point, 22, 5);
    copy(vertex, point, 23, 4);

    copy(vertex, point, 24, 7);
    copy(vertex, point, 25, 3);
    copy(vertex, point, 26, 0);
    copy(vertex, point, 27, 7);
    copy(vertex, point, 28, 0);
    copy(vertex, point, 29, 6);

    copy(vertex, point, 30, 7);
    copy(vertex, point, 31, 4);
    copy(vertex, point, 32, 2);
    copy(vertex, point, 33, 7);
    copy(vertex, point, 34, 2);
    copy(vertex, point, 35, 3);

    auto buffer = generateGLBuffer(vertex, 3, 0);

 
    //glwidget_->setColor(QVector4D(0.5f, 1.f, 0.5f, 1.f));
    //glDisable(GL_DEPTH_TEST);
    //drawArrays(GL_POINTS, buffer);
    drawArrays(GL_TRIANGLES, buffer);
    //glEnable(GL_DEPTH_TEST);
    //if(fill) {
    //    glutSolidCube(1.0);
    //}
    //else {
    //    glutWireCube(1.0);
    //    if(show_heading) {
    //        glBegin(GL_LINES);
    //            glVertex3f(0.f, 0.f, .5f);
    //            glVertex3f(.5f, 0.f, .5f);
    //        glEnd();
    //    }
    //}
}
#else
void Renderer::drawBoundingBox(const Eigen::Vector3f &center, const Eigen::Vector3f &lwh,
                               const float heading, const bool fill, const bool show_heading) {
  GLPushGuard pg;
  glTranslatef(center.x(), center.y(), center.z());
  glRotatef(heading / M_PI * 180.f, 0, 0, 1);
  glScalef(lwh.x(), lwh.y(), lwh.z());
  if (fill) {
    glutSolidCube(1.0);
  } else {
    glutWireCube(1.0);
    if (show_heading) {
      glBegin(GL_LINES);
      glVertex3f(0.f, 0.f, .5f);
      glVertex3f(.5f, 0.f, .5f);
      glEnd();
    }
  }
}
#endif

void Renderer::drawConvexCylinder(const std::vector<Eigen::Vector2f> &polygon, const float height) {
  if (polygon.size() < 3) {
    return;
  }
  auto vertex = generateVertex(polygon);
#ifdef __aarch64__
  auto indice = triangulate(polygon);
  auto buffer = generateGLBuffer(vertex, 2, 0, indice);
  drawArrays(GL_LINE_LOOP, buffer);
  {
    GLPushGuard pg;
    drawArrays(GL_LINE_LOOP, buffer);
  }

  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
#else
  auto buffer = generateGLBuffer(vertex, 2, 0);
  drawArrays(GL_LINE_LOOP, buffer);
  {
    GLPushGuard pg;
    glTranslatef(0.f, 0.f, height);
    drawArrays(GL_LINE_LOOP, buffer);
  }

  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  glBegin(GL_QUAD_STRIP);
  for (const auto &pt : polygon) {
    glVertex3f(pt.x(), pt.y(), 0.f);
    glVertex3f(pt.x(), pt.y(), height);
  }
  glVertex3f(polygon.front().x(), polygon.front().y(), 0.f);
  glVertex3f(polygon.front().x(), polygon.front().y(), height);
  glEnd();
#endif
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

float Renderer::drawText(const Eigen::VectorXf &pos, const std::string &text, const int font_size,
                         const bool bold) {
  if (pos.rows() < 2 || pos.rows() > 3) {
    return std::numeric_limits<float>::max();
  }

  auto &pen = (bold ? global_data_->font_bold_ : global_data_->font_normal_);
  if (pen) {
    pen->FaceSize(font_size);
    if (pos.rows() == 2) {
      glPushAttrib(GL_ALL_ATTRIB_BITS);
      glRasterPos2f(pos.x(), pos.y());
      auto new_pos = pen->Render(text.c_str(), -1);
      glPopAttrib();
      return new_pos.Xf() + pos.x();
    } else {
      glPushAttrib(GL_ALL_ATTRIB_BITS);
      glRasterPos3f(pos.x(), pos.y(), pos.z());
      auto new_pos = pen->Render(text.c_str(), -1);
      glPopAttrib();
      return new_pos.Xf() + pos.x();
    }
  } else {
    void *font = nullptr;
    if (font_size >= 24) {
      font = GLUT_BITMAP_TIMES_ROMAN_24;
    } else if (font_size >= 18) {
      font = GLUT_BITMAP_HELVETICA_18;
    } else if (font_size <= 10) {
      font = GLUT_BITMAP_HELVETICA_10;
    } else {
      font = GLUT_BITMAP_HELVETICA_12;
    }

    glPushAttrib(GL_ALL_ATTRIB_BITS);
    if (pos.rows() == 2) {
      glRasterPos2f(pos.x(), pos.y());
    } else {
      glRasterPos3f(pos.x(), pos.y(), pos.z());
    }
    glutBitmapString(font, (const unsigned char *)(text.c_str()));
    glPopAttrib();

    int width = 0;
    for (const auto &c : text) {
      width += glutBitmapWidth(font, c);
    }

    return pos.x() + width;
  }
}

void Renderer::drawIcon(const Eigen::Vector3f &pos, const QIcon &icon, const int size) {
  int viewport[4];
  glGetIntegerv(GL_VIEWPORT, viewport);

  glPushMatrix();
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glOrtho(0.0, viewport[2], 0.0, viewport[3], -1, 2000);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glDisable(GL_DEPTH_TEST);

  const auto p = global_data_->camera_->getScreenPoint({pos.x(), pos.y(), pos.z()});
  const float half_size = size*0.5;

  glBegin(GL_QUADS);
    glVertex2f(p.x - half_size, p.y - half_size);
    glVertex2f(p.x + half_size, p.y - half_size);
    glVertex2f(p.x + half_size, p.y + half_size);
    glVertex2f(p.x - half_size, p.y + half_size);
  glEnd();

  glEnable(GL_DEPTH_TEST);
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
}

std::vector<GLTextureDesc> Renderer::loadTextureDesc(const std::string &path) const {
  // load image and convert to RGBA format
  auto image = cv::imread(path, cv::IMREAD_UNCHANGED);
  // RETURN_VAL_IF(image.empty(), {});
  switch(image.channels()) {
    case 1:
      cv::cvtColor(image, image, CV_GRAY2RGBA);
      break;
    case 3:
      cv::cvtColor(image, image, CV_BGR2RGBA);
      break;
    case 4:
      cv::cvtColor(image, image, CV_BGRA2RGBA);
      break;
    default:
      LOG(WARNING) << "Not supported channels: " << image.channels();
      return {};
  }

  // !!! large image may core dumped in cv::flip, do vertical flipping manually
  for (int row = 0; row < image.rows / 2; ++row) {
    int swap_row = image.rows - row - 1;
    for (int col = 0; col < image.cols; ++col) {
      auto t = image.at<cv::Vec4b>(row, col);
      image.at<cv::Vec4b>(row, col) = image.at<cv::Vec4b>(swap_row, col);
      image.at<cv::Vec4b>(swap_row, col) = t;
    }
  }

  const static int MAX_ROWS_COLS = 32768;
  const static int MAX_BYTES = 1000000000;  // near 1GB
  const static int CLIP_ROWS_COLS = 10000;

  if (image.cols <= MAX_ROWS_COLS && image.rows <= MAX_ROWS_COLS &&
      image.cols * image.rows * image.channels() <= MAX_BYTES) {
    GLTextureDesc texture_desc;
    texture_desc.image_ = image;
    texture_desc.roi_ = QRect(0, 0, image.cols, image.rows);
    return {texture_desc};
  } else {
    std::vector<GLTextureDesc> texture_descs;
    int clip_rows = image.rows / CLIP_ROWS_COLS;
    if (clip_rows * CLIP_ROWS_COLS < image.rows) {
      ++clip_rows;
    }
    int clip_cols = image.cols / CLIP_ROWS_COLS;
    if (clip_cols * CLIP_ROWS_COLS < image.cols) {
      ++clip_cols;
    }

    for (int clip_row = 0; clip_row < clip_rows; ++clip_row) {
      int y = clip_row * CLIP_ROWS_COLS;
      int height = (clip_row == clip_rows - 1 ? image.rows - CLIP_ROWS_COLS * (clip_rows - 1)
                                              : CLIP_ROWS_COLS);

      for (int clip_col = 0; clip_col < clip_cols; ++clip_col) {
        int x = clip_col * CLIP_ROWS_COLS;
        int width = (clip_col == clip_cols - 1 ? image.cols - CLIP_ROWS_COLS * (clip_cols - 1)
                                               : CLIP_ROWS_COLS);

        auto image_roi = image(cv::Rect(x, y, width, height));

        GLTextureDesc texture_desc;
        texture_desc.image_ = image_roi;
        texture_desc.roi_ = QRect(x, y, width, height);
        texture_descs.push_back(texture_desc);
      }
    }

    return texture_descs;
  }
}

std::vector<GLTexture> Renderer::generateTexture(
    const std::vector<GLTextureDesc> &texture_descs) const {
  std::vector<GLTexture> textures;
  for (const auto &desc : texture_descs) {
    auto qimg = QImage(desc.image_.data, desc.image_.cols, desc.image_.rows, desc.image_.step,
                       QImage::Format_RGBA8888);
    GLTexture texture;
    texture.texture_.reset(new QOpenGLTexture(qimg));
    texture.roi_ = desc.roi_;
    textures.push_back(texture);
  }
  return textures;
}

void Renderer::renderTexture(const GLTexture &texture, const QPointF &offset,
                             const float resolution) {
  glEnable(GL_TEXTURE_2D);
  texture.texture_->bind();
  glBegin(GL_QUADS);
  glTexCoord2f(0, 0);
  glVertex2f(texture.roi_.x() * resolution + offset.x(),
             texture.roi_.y() * resolution + offset.y());
  glTexCoord2f(0, 1);
  glVertex2f(texture.roi_.x() * resolution + offset.x(),
             (texture.roi_.y() + texture.roi_.height()) * resolution + offset.y());
  glTexCoord2f(1, 1);
  glVertex2f((texture.roi_.x() + texture.roi_.width()) * resolution + offset.x(),
             (texture.roi_.y() + texture.roi_.height()) * resolution + offset.y());
  glTexCoord2f(1, 0);
  glVertex2f((texture.roi_.x() + texture.roi_.width()) * resolution + offset.x(),
             texture.roi_.y() * resolution + offset.y());
  glEnd();
  texture.texture_->release();
  glDisable(GL_TEXTURE_2D);
}

void Renderer::renderTextureViewFacing(const GLTexture &texture, const Eigen::Vector3f &pos, const int size) {
  int viewport[4];
  glGetIntegerv(GL_VIEWPORT, viewport);

  glPushMatrix();
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glOrtho(0.0, viewport[2], 0.0, viewport[3], -1, 2000);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  // glDisable(GL_DEPTH_TEST);

  const auto p = global_data_->camera_->getScreenPoint({pos.x(), pos.y(), pos.z()});
  const float half_size = size*0.5;

  glEnable(GL_TEXTURE_2D);
  texture.texture_->bind();
  glBegin(GL_QUADS);
    glTexCoord2f(0, 0);
    glVertex2f(p.x - half_size, p.y - half_size);
    glTexCoord2f(1, 0);
    glVertex2f(p.x + half_size, p.y - half_size);
    glTexCoord2f(1, 1);
    glVertex2f(p.x + half_size, p.y + half_size);
    glTexCoord2f(0, 1);
    glVertex2f(p.x - half_size, p.y + half_size);
  glEnd();
  texture.texture_->release();
  glDisable(GL_TEXTURE_2D);

  // glEnable(GL_DEPTH_TEST);
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
}

Eigen::MatrixXf Renderer::generateVertex(const std::vector<Eigen::VectorXf> &points,
                                         const std::vector<Eigen::VectorXf> &colors) const {
  const auto dim_points = checkDimension(points);
  const auto dim_colors = checkDimension(colors);
  if (dim_points <= 0) {
    return Eigen::MatrixXf();
  }

  Eigen::MatrixXf vertex(dim_points + dim_colors, points.size());
  for (size_t i = 0; i < points.size(); ++i) {
    const auto &p = points[i];
    for (int d = 0; d < dim_points; ++d) {
      vertex(d, i) = p[d];
    }
    if (dim_colors > 0) {
      const auto &c = colors[i];
      for (int d = 0; d < dim_colors; ++d) {
        vertex(d + dim_points, i) = c[d];
      }
    }
  }

  return vertex;
}

Eigen::MatrixXf Renderer::generateVertex(const std::vector<Eigen::Vector2f> &points,
                                         const std::vector<Eigen::VectorXf> &colors) const {
  if (points.empty()) {
    return Eigen::MatrixXf();
  }

  const int dim_pos = 2;
  const auto dim_color = checkDimension(colors);

  Eigen::MatrixXf vertex(dim_pos + dim_color, points.size());
  for (size_t i = 0; i < points.size(); ++i) {
    const auto &p = points[i];
    for (int d = 0; d < dim_pos; ++d) {
      vertex(d, i) = p[d];
    }
    if (dim_color > 0) {
      const auto &c = colors[i];
      for (int d = 0; d < dim_color; ++d) {
        vertex(d + dim_pos, i) = c[d];
      }
    }
  }

  return vertex;
}

GLBuffer Renderer::generateGLBuffer(const Eigen::MatrixXf &vertex, const uint8_t dim_points,
                                    const uint8_t dim_colors,
                                    const std::vector<unsigned int> &indices) {
  if (vertex.rows() <= 0 || vertex.cols() <= 0 || vertex.size() <= 0 || dim_points == 0) {
    return GLBuffer();
  }

  GLBuffer buffer;
  buffer.count_vertex = vertex.cols();
  buffer.count_index = indices.size();
  buffer.vao.reset(new QOpenGLVertexArrayObject());
  buffer.vbo.reset(new QOpenGLBuffer(QOpenGLBuffer::Type::VertexBuffer));
  buffer.vao->create();
  buffer.vao->bind();
  buffer.vbo->create();
  buffer.vbo->bind();
  buffer.vbo->allocate(vertex.data(), sizeof(float) * vertex.size());
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, dim_points, GL_FLOAT, GL_FALSE,
                        sizeof(float) * (dim_points + dim_colors), nullptr);
  if (dim_colors > 0) {
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, dim_colors, GL_FLOAT, GL_FALSE,
                          sizeof(float) * (dim_points + dim_colors),
                          (void *)(sizeof(float) * dim_points));
  }
  if (!indices.empty()) {
    buffer.ibo.reset(new QOpenGLBuffer(QOpenGLBuffer::Type::IndexBuffer));
    buffer.ibo->create();
    buffer.ibo->bind();
    buffer.ibo->allocate(indices.data(), sizeof(unsigned int) * indices.size());
    buffer.ibo->release();
  }
  buffer.vbo->release();
  buffer.vao->release();

  return buffer;
}

GLBuffer Renderer::generateGLBuffer(const std::vector<std::vector<Eigen::Vector2f>> &polygons,
                                    const std::vector<Eigen::VectorXf> &colors) {
  const auto dim_colors = checkDimension(colors);
  int size_vertex = 0;
  int size_indices = 0;
  std::vector<unsigned int> triangulated_indices;
  for (const auto &polygon : polygons) {
    auto indices = triangulate(polygon);
    for (const auto index : indices) {
      triangulated_indices.push_back(index + size_vertex);
    }
    size_vertex += polygon.size();
    size_indices += indices.size();
  }

  if (size_vertex <= 0 || size_indices <= 0) {
    return GLBuffer();
  }

  Eigen::MatrixXf vertex(2 + dim_colors, size_vertex);
  auto p = vertex.data();
  for (size_t polygon_no = 0; polygon_no < polygons.size(); ++polygon_no) {
    const auto &polygon = polygons[polygon_no];
    const auto &color = colors[polygon_no];
    for (const auto &pt : polygon) {
      *p++ = pt.x();
      *p++ = pt.y();
      for (int i = 0; i < dim_colors; ++i) {
        *p++ = color[i];
      }
    }
  }

  GLBuffer buffer;
  buffer.count_vertex = size_vertex;
  buffer.count_index = size_indices;
  buffer.vao.reset(new QOpenGLVertexArrayObject());
  buffer.vbo.reset(new QOpenGLBuffer(QOpenGLBuffer::Type::VertexBuffer));
  buffer.ibo.reset(new QOpenGLBuffer(QOpenGLBuffer::Type::IndexBuffer));
  buffer.vao->create();
  buffer.vao->bind();
  buffer.vbo->create();
  buffer.vbo->bind();
  buffer.vbo->allocate(vertex.data(), sizeof(float) * vertex.size());
  buffer.ibo->create();
  buffer.ibo->bind();
  buffer.ibo->allocate(triangulated_indices.data(),
                       sizeof(unsigned int) * triangulated_indices.size());
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * (2 + dim_colors), nullptr);
  if (dim_colors > 0) {
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, dim_colors, GL_FLOAT, GL_FALSE, sizeof(float) * (2 + dim_colors),
                          (void *)(sizeof(float) * 2));
  }
  buffer.vao->release();

  return buffer;
}

void Renderer::drawArrays(const GLenum mode, const std::vector<Eigen::VectorXf> &points,
                          const std::vector<Eigen::VectorXf> &colors) {
  const auto dim_points = checkDimension(points);
  const auto dim_colors = checkDimension(colors);
  if (dim_points <= 0) {
    return;
  }

  auto vertex = generateVertex(points, colors);
#ifdef __aarch64__
  auto indice = triangulate(points);
  auto buffer = generateGLBuffer(vertex, dim_points, dim_colors, indice);
  drawArrays(mode, buffer);
}

#else
  auto buffer = generateGLBuffer(vertex, dim_points, dim_colors);
  drawArrays(mode, buffer);
}
void Renderer::drawArrays(const GLenum mode, const Eigen::MatrixXf &vertex,
                          const uint8_t dim_points, const uint8_t dim_colors) {
  auto buffer = generateGLBuffer(vertex, dim_points, dim_colors);
  drawArrays(mode, buffer);
}
#endif


void Renderer::drawArrays(const GLenum mode, GLBuffer &buffer) {
  if (!buffer.vao || !buffer.vbo) {
    return;
  }

  buffer.vao->bind();
  buffer.vbo->bind();
  glDrawArrays(mode, 0, buffer.count_vertex);
  buffer.vbo->release();
  buffer.vao->release();
}

void Renderer::drawElements(const GLenum mode, GLBuffer &buffer) {
  if (!buffer.vao || !buffer.vbo || !buffer.ibo) {
    return;
  }

  buffer.vao->bind();
  buffer.vbo->bind();
  buffer.ibo->bind();
  glDrawElements(mode, buffer.count_index, GL_UNSIGNED_INT, nullptr);
  buffer.ibo->release();
  buffer.vbo->release();
  buffer.vao->release();
}

std::vector<Eigen::Vector2f> Renderer::generateLineStripPolygon(
    const std::vector<Eigen::Vector2f> &points, const float width) {
  if (points.size() < 2) {
    return {};
  }

  std::vector<Eigen::Vector2f> left, right;
  const auto &half_width = width * 0.5f;
  for (size_t i = 0; i < points.size(); ++i) {
    auto beg = (i == 0 ? 0 : i - 1);
    auto end = (i == points.size() - 1 ? points.size() - 1 : i + 1);
    const auto angle =
        std::atan2(points[end].y() - points[beg].y(), points[end].x() - points[beg].x());
    const auto sin_angle = half_width * std::sin(angle);
    const auto cos_angle = half_width * std::cos(angle);
    left.emplace_back(points[i].x() - sin_angle, points[i].y() + cos_angle);
    right.emplace_back(points[i].x() + sin_angle, points[i].y() - cos_angle);
  }
  left.insert(left.end(), right.rbegin(), right.rend());

  return left;
}

std::vector<Eigen::Vector2f> Renderer::generateLineStripQuads(
    const std::vector<Eigen::Vector2f> &points, const float width) {
  if (points.size() < 2) {
    return {};
  }

  std::vector<Eigen::Vector2f> points_quads;
  const auto &half_width = width * 0.5f;
  for (size_t i = 0; i < points.size(); ++i) {
    auto beg = (i == points.size() - 1 ? i - 1 : i);
    auto end = (i == points.size() - 1 ? i : i + 1);
    const auto angle =
        std::atan2(points[end].y() - points[beg].y(), points[end].x() - points[beg].x());
    const auto sin_angle = half_width * std::sin(angle);
    const auto cos_angle = half_width * std::cos(angle);
    points_quads.emplace_back(points[i].x() - sin_angle, points[i].y() + cos_angle);
    points_quads.emplace_back(points[i].x() + sin_angle, points[i].y() - cos_angle);
  }

  return points_quads;
}

std::vector<std::vector<Eigen::Vector2f>> Renderer::generateDashedLineStripPolygons(
    const std::vector<Eigen::Vector2f> &points, const float width, const float length_segment,
    const float ratio) {
  if (points.size() < 2) {
    return {};
  }

  const float length_real = length_segment * ratio;
  float length_acc = 0.f;
  std::vector<std::vector<Eigen::Vector2f>> segments;
  std::vector<Eigen::Vector2f> segment;
  segment.push_back(points.front());
  for (auto it = points.cbegin() + 1; it != points.cend();) {
    segment.push_back(*it);

    const auto &pt_last = *(it - 1);
    const auto &pt = *it;
    length_acc += std::hypot(pt.x() - pt_last.x(), pt.y() - pt_last.y());
    ++it;

    if (length_acc >= length_real) {
      if (segment.size() >= 2) {
        segments.push_back(generateLineStripPolygon(segment, width));
        segment.clear();
      }

      while (length_acc < length_segment && it != points.cend()) {
        const auto &pt_last = *(it - 1);
        const auto &pt = *it;
        length_acc += std::hypot(pt.x() - pt_last.x(), pt.y() - pt_last.y());
        ++it;
      }

      length_acc = 0.f;
    }
  }

  if (segment.size() >= 2) {
    segments.push_back(generateLineStripPolygon(segment, width));
  }

  return segments;
}

int Renderer::checkDimension(const std::vector<Eigen::VectorXf> &vertex) const {
  if (vertex.empty()) {
    return 0;
  }

  const auto &front = vertex.front();
  const int dim = front.rows();

  for (const auto &v : vertex) {
    if (v.rows() != dim) {
      return 0;
    }
  }

  return dim;
}

std::vector<unsigned int> Renderer::triangulate(const std::vector<Eigen::Vector2f> &polygon) const {
  std::vector<std::vector<Eigen::Vector2f>> polygon_with_hole;
  polygon_with_hole.push_back(polygon);
  return mapbox::earcut<unsigned int>(polygon_with_hole);
}

#ifdef __aarch64__
std::vector<unsigned int> Renderer::triangulate(const std::vector<Eigen::VectorXf> &points) const {
    //std::cout<<" triangulate points "<<points.size()<<std::endl;
    std::vector<std::vector<Eigen::Vector2f>> tmp;
    std::vector<Eigen::Vector2f> tmp_;
    for(int i = 0; i < points.size(); i++) {
        const auto &p = points[i];
        Eigen::Vector2f t;
        t[0] = p[0];
        t[1] = p[1];
	tmp_.push_back(t);
    }

    tmp.push_back(tmp_);
    return mapbox::earcut<unsigned int>(tmp);

}
#endif

}  // namespace airi
}  // namespace crdc
