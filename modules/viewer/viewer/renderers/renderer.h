#pragma once

#include <cyber/cyber.h>
#include <Eigen/Eigen>
#include <QOpenGLBuffer>
#include <QOpenGLFunctions>
#include <QOpenGLTexture>
#include <QOpenGLVertexArrayObject>
#include <memory>
#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs/legacy/constants_c.h>
#include <opencv2/imgproc/types_c.h>
#include <string>
#include <GL/gl.h>
#include <math.h>
#include <glog/logging.h>

namespace geometry_msgs {
class TransformStamped;
}

namespace crdc {
namespace airi {

#ifdef __aarch64__

static GLenum glCheckError_(const char *file, int line)
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
namespace viewer {
class Color;
}
class GlobalData;

struct GLPushGuard {
  GLPushGuard() { glPushMatrix(); }
  ~GLPushGuard() { glPopMatrix(); }
};

struct GLBuffer {
  std::shared_ptr<QOpenGLVertexArrayObject> vao;
  std::shared_ptr<QOpenGLBuffer> vbo, ibo;
  size_t count_vertex;
  size_t count_index;
};

struct GLBufferWithTrans {
  GLBuffer buffer;
  std::string frame_id;
  uint64_t utime;
};

struct GLTextureDesc {
  cv::Mat image_;
  QRect roi_;
};

struct GLTexture {
  std::shared_ptr<QOpenGLTexture> texture_;
  QRect roi_;
};

class Renderer : protected QOpenGLFunctions {
 public:
  Renderer();
  virtual ~Renderer() {}

 public:
  virtual std::string name() const { return ""; }

  virtual bool enabled() const { return false; }

  virtual void initialize() { initializeOpenGLFunctions(); }

  virtual void render() {}

#ifdef __aarch64__

  void bot_quat_to_roll_pitch_yaw (const double q[4], double rpy[3]) 
  {
    double roll_a = 2 * (q[0]*q[1] + q[2]*q[3]);
    double roll_b = 1 - 2 * (q[1]*q[1] + q[2]*q[2]);
    rpy[0] = atan2 (roll_a, roll_b);

    double pitch_sin = 2 * (q[0]*q[2] - q[3]*q[1]);
    rpy[1] = asin (pitch_sin);

    double yaw_a = 2 * (q[0]*q[3] + q[1]*q[2]);
    double yaw_b = 1 - 2 * (q[2]*q[2] + q[3]*q[3]);
    rpy[2] = atan2 (yaw_a, yaw_b);
  }
#endif
  virtual void loadConfigPost() {}

  // tools
 protected:
  void setColor(const viewer::Color &color);

  // void transform(const std::string &target_frame_id, const std::string &source_frame_id = "global", const uint64_t utime = 0);

  // higher level APIs
 protected:
  void drawPoints(const std::vector<Eigen::VectorXf> &points,
                  const std::vector<Eigen::VectorXf> &colors = {});

  void drawLines(const std::vector<Eigen::VectorXf> &points,
                 const std::vector<Eigen::VectorXf> &colors = {});

  void drawLineStrip(const std::vector<Eigen::VectorXf> &points,
                     const std::vector<Eigen::VectorXf> &colors = {});

  void drawLineLoop(const std::vector<Eigen::VectorXf> &points,
                    const std::vector<Eigen::VectorXf> &colors = {});

  void drawArrow(const Eigen::Vector3f &from, const Eigen::Vector3f &to, const float scale = 0.3f);

  void drawTriangles(const std::vector<Eigen::VectorXf> &points,
                     const std::vector<Eigen::VectorXf> &colors = {});

  void drawEllipse(const Eigen::Vector2f &center, const float radius_x, const float radius_y,
                   const float heading, const bool fill = true, const float step = M_PI / 180.f);

  void drawCircle(const Eigen::Vector2f &center, const float radius, const bool fill = true,
                  const float step = M_PI / 180.f);

  void drawArch(const Eigen::Vector2f &center, const float radius, const float beg = 0.f,
                const float end = M_PI * 2, const bool fill = true,
                const float step = M_PI / 180.f);

  void drawRect(const Eigen::Vector2f &left_top, const Eigen::Vector2f &right_bottom);

  void drawRect(const Eigen::Vector4f &xywh);
#ifdef __aarch64__
  void drawRect(const Eigen::MatrixXf& point);
#endif

  void drawPolygon(const std::vector<Eigen::Vector2f> &polygon);

  void drawLineAsQuad(const Eigen::Vector2f &start, const Eigen::Vector2f &end, const float width);

  void drawLineStripPolygon(const std::vector<Eigen::Vector2f> &points, const float width);

  void drawLineStripQuads(const std::vector<Eigen::Vector2f> &points, const float width);

  void drawSphere(const Eigen::Vector3f &center, const float radius);

  void drawBoundingBox(const Eigen::Vector3f &center, const Eigen::Vector3f &lwh,
                       const float heading, const bool fill = false,
                       const bool show_heading = true);

  void drawConvexCylinder(const std::vector<Eigen::Vector2f> &polygon, const float height);

  float drawText(const Eigen::VectorXf &pos, const std::string &text, const int font_size,
                 const bool bold = false);

  void drawIcon(const Eigen::Vector3f &pos, const QIcon &icon, const int size);

  std::vector<GLTextureDesc> loadTextureDesc(const std::string &path) const;

  std::vector<GLTexture> generateTexture(const std::vector<GLTextureDesc> &texture_descs) const;

  void renderTexture(const GLTexture &texture, const QPointF &offset, const float resolution);

  void renderTextureViewFacing(const GLTexture &texture, const Eigen::Vector3f &pos, const int size);

  // lower level APIs
 protected:
  Eigen::MatrixXf generateVertex(const std::vector<Eigen::VectorXf> &points,
                                 const std::vector<Eigen::VectorXf> &colors = {}) const;

  Eigen::MatrixXf generateVertex(const std::vector<Eigen::Vector2f> &points,
                                 const std::vector<Eigen::VectorXf> &colors = {}) const;

  GLBuffer generateGLBuffer(const Eigen::MatrixXf &vertex, const uint8_t dim_points,
                            const uint8_t dim_colors,
                            const std::vector<unsigned int> &indices = {});

  GLBuffer generateGLBuffer(const std::vector<std::vector<Eigen::Vector2f>> &polygons,
                            const std::vector<Eigen::VectorXf> &colors = {});

  void drawArrays(const GLenum mode, const std::vector<Eigen::VectorXf> &points,
                  const std::vector<Eigen::VectorXf> &colors = {});

  void drawArrays(const GLenum mode, const Eigen::MatrixXf &vertex, const uint8_t dim_points,
                  const uint8_t dim_colors);

  void drawArrays(const GLenum mode, GLBuffer &buffer);

  void drawElements(const GLenum mode, GLBuffer &buffer);

  std::vector<Eigen::Vector2f> generateLineStripPolygon(const std::vector<Eigen::Vector2f> &points,
                                                        const float width);

  std::vector<Eigen::Vector2f> generateLineStripQuads(const std::vector<Eigen::Vector2f> &points,
                                                      const float width);

  std::vector<std::vector<Eigen::Vector2f>> generateDashedLineStripPolygons(
      const std::vector<Eigen::Vector2f> &points, const float width, const float length_segment,
      const float ratio = 0.5f);

  // inner functions
 private:
  int checkDimension(const std::vector<Eigen::VectorXf> &vertex) const;

  std::vector<unsigned int> triangulate(const std::vector<Eigen::Vector2f> &polygon) const;
#ifdef __aarch64__
  std::vector<unsigned int> triangulate(const std::vector<Eigen::VectorXf> &points) const;
#endif

 protected:
  GlobalData *global_data_;
};

}  // namespace airi
}  // namespace crdc
