#include "viewer/widgets/slider.h"
#include <iomanip>
#include <sstream>
#include <cmath>

namespace crdc {
namespace airi {

Slider::Slider(const QString &text, const int precision, const double min, const double max,
               const double init_value, const std::function<void(double)> &callback)
    : label_name_(new QLabel(text)),
      label_value_(new QLabel()),
      slider_(new QSlider(Qt::Horizontal)),
      hbox_(new QHBoxLayout()),
      precision_(precision),
      multiplier_set_(pow(10, precision)),
      multiplier_get_(pow(10, -precision)),
      callback_(callback) {
  slider_->setMinimum(min * multiplier_set_);
  slider_->setMaximum(max * multiplier_set_);
  slider_->setValue(init_value * multiplier_set_);
  std::stringstream ss;
  ss << std::setprecision(precision_) << std::fixed << init_value;
  label_value_->setText(QString::fromStdString(ss.str()));
  QObject::connect(slider_, static_cast<void (QSlider::*)(int)>(&QSlider::valueChanged),
                   [&](int value) {
                     double val = value * multiplier_get_;
                     callback_(val);
                     std::stringstream ss;
                     ss << std::setprecision(precision_) << std::fixed << val;
                     label_value_->setText(QString::fromStdString(ss.str()));
                   });

  hbox_->addWidget(label_name_);
  hbox_->addWidget(slider_);
  hbox_->addWidget(label_value_);
  this->setLayout(hbox_);
  hbox_->setSpacing(0);
  hbox_->setMargin(3);
}

}  // namespace airi
}  // namespace crdc
