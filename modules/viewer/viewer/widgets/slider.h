#pragma once

#include <QLabel>
#include <QLayout>
#include <QSlider>
#include <QString>
#include <functional>

namespace crdc {
namespace airi {

class Slider : public QWidget {
 public:
  Slider(const QString &text, const int precision, const double min, const double max,
         const double init_value, const std::function<void(double)> &callback);

 protected:
  QLabel *label_name_;
  QLabel *label_value_;
  QSlider *slider_;
  QHBoxLayout *hbox_;
  int precision_;
  double multiplier_set_;
  double multiplier_get_;
  const std::function<void(double)> callback_;
};

}  // namespace airi
}  // namespace crdc
