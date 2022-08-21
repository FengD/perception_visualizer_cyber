/**
 * @file
 * @ingroup TYPE
 * @ingroup PB
 *
 * @brief Image conversion between protobuf message and OpenCV
 *
 * @note protobuf messages are jpeg compressed
 */

#pragma once

#include <string>
#include <vector>
#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs/legacy/constants_c.h>
#include <opencv2/imgproc/types_c.h>
#include "cyber/common/log.h"
#include "cyber/sensor_proto/image.pb.h"

namespace crdc {
namespace airi {
namespace util {
static inline bool cv_decoder(const crdc::airi::Image2 &proto_image, cv::Mat *image) {
  std::vector<uint8_t> compressed_buffer(proto_image.data().begin(), proto_image.data().end());
  *image = cv::imdecode(compressed_buffer, CV_LOAD_IMAGE_UNCHANGED);

  // decoded 3 channel image is in order bgr
  if (proto_image.type() == std::to_string(crdc::airi::RGB8)) {
    cv::cvtColor(*image, *image, CV_BGR2RGB);
  }
  return true;
}

/**
 * @brief convert Image from cv::Mat to pb message
 *        not use now.
 */
// static bool toProto(
//     const cv::Mat &image, crdc::airi::Image2 *proto_image,
//     const crdc::airi::PixelFormat &type = crdc::airi::BGR8,
//     const crdc::airi::Image2_Compression &compression = crdc::airi::Image2_Compression_JPEG,
//     const int quality = 75, const uint64_t utime = 0, const std::string& frame_id = "") {
//   RETURN_VAL_IF(image.empty(), false);
//   RETURN_VAL_IF(type == crdc::airi::MONO8 && image.channels() != 1, false);
//   RETURN_VAL_IF(
//       (type == crdc::airi::RGB8 || type == crdc::airi::BGR8) &&
//           (image.channels() != 3),
//       false);

//   proto_image->set_height(image.rows);
//   proto_image->set_width(image.cols);
//   proto_image->set_step(image.step);
//   // proto_image->set_type(type);
//   proto_image->set_compression(compression);
//   proto_image->mutable_header()->set_camera_timestamp(utime);
//   proto_image->mutable_header()->set_frame_id(frame_id);

//   if (compression == crdc::airi::Image2_Compression_RAW) {
//     auto data = proto_image->mutable_data();
//     data->resize(image.step * image.rows);
//     memcpy(const_cast<char *>(data->data()), image.data, image.step * image.rows);
//     return true;
//   } else {
//     std::string ext;
//     if (compression == crdc::airi::Image2_Compression_JPEG) {
//       ext = ".jpg";
//     } else {
//       LOG(ERROR) << "Unexpected image compression type: " << compression;
//       return false;
//     }

//     // compress
//     std::vector<int> params;
//     params.resize(3, 0);
//     params[0] = CV_IMWRITE_JPEG_QUALITY;
//     params[1] = quality;
//     std::vector<uint8_t> compressed_buffer;
//     if (type == crdc::airi::RGB8) {
//       // imencode needs bgr image input
//       cv::Mat bgr;
//       cv::cvtColor(image, bgr, CV_RGB2BGR);
//       RETURN_VAL_IF(!cv::imencode(ext, bgr, compressed_buffer, params), false);
//     } else {
//       RETURN_VAL_IF(!cv::imencode(ext, image, compressed_buffer, params), false);
//     }

//     // fill proto data
//     auto data = proto_image->mutable_data();
//     data->resize(compressed_buffer.size());
//     memcpy(const_cast<char *>(data->data()), compressed_buffer.data(), compressed_buffer.size());
//     return true;
//   }
// }

/**
 * @brief convert Image from pb message to cv::Mat
 * @param proto_image
 * @param image
 * @param decoder
 */
static bool convert_from_proto(const crdc::airi::Image2 &proto_image, cv::Mat *image,
                      const std::function<bool(const crdc::airi::Image2 &, cv::Mat *)> &decoder) {
  if (proto_image.compression() == crdc::airi::Image2_Compression_RAW) {
    int type;
    if (proto_image.type() == std::to_string(crdc::airi::MONO8)) {
      type = CV_8UC1;
      *image = cv::Mat(proto_image.height(), proto_image.width(), type,
                     const_cast<char *>(proto_image.data().data()), proto_image.step())
                 .clone();
    } else if (proto_image.type() == std::to_string(crdc::airi::RGB8) ||
               proto_image.type() == std::to_string(crdc::airi::BGR8)) {
      type = CV_8UC3;
      *image = cv::Mat(proto_image.height(), proto_image.width(), type,
                     const_cast<char *>(proto_image.data().data()), proto_image.step())
                 .clone();
    } else if (proto_image.type() == std::string("NV12")) {
      cv::Mat nv12(proto_image.height() * 3 / 2, proto_image.width(), CV_8UC1, (void*)proto_image.data().data());
      cv::Mat rgb(proto_image.height(), proto_image.width(), CV_8UC3);
      cv::cvtColor(nv12, rgb, cv::COLOR_YUV2RGB_NV12);
      *image = rgb.clone();
    } else if (proto_image.type() == std::string("Y")) {
      cv::Mat y(proto_image.height(), proto_image.width(), CV_8UC1, (void*)proto_image.data().data());
      cv::Mat rgb(proto_image.height(), proto_image.width(), CV_8UC3);
      cv::cvtColor(y, rgb, cv::COLOR_GRAY2RGB);
      *image = rgb.clone();
    } else {
      LOG(ERROR) << "Unexpected image type: " << proto_image.type();
      return false;
    }
    return true;
  }
  return decoder(proto_image, image);
}

/**
 * @brief convert Image from pb message to cv::Mat, cv_decoder(default) wrapper
 * @param proto_image
 * @param image
 */
static bool convert_from_proto(const crdc::airi::Image2 &proto_image, cv::Mat *image) {
  return convert_from_proto(proto_image, image, cv_decoder);
}

}  // namespace util
}  // namespace airi
}  // namespace crdc
