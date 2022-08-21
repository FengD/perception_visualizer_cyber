#pragma once

#include <QColor>
#include <QPushButton>
#include <functional>

namespace crdc {
namespace airi {

class ColorButton : public QPushButton {
 public:
  ColorButton(const QColor &color, const std::function<void(QColor &color)> &callback);

 protected:
  void paintEvent(QPaintEvent *e) override;

 protected:
  QColor color_;
  const std::function<void(QColor &color)> callback_;
};

}  // namespace airi
}  // namespace crdc
