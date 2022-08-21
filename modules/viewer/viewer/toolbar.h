#pragma once

#include <QWidget>
#include <memory>
#include "cyber/sensor_proto/localization.pb.h"

namespace crdc {
namespace airi {

class Toolbar : public QWidget {
 public:
  Toolbar();

 protected:
  std::shared_ptr<crdc::airi::LocalizationEstimate> pose_prev_;
};

}  // namespace airi
}  // namespace crdc
