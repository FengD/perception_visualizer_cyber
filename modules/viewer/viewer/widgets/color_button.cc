#include "viewer/widgets/color_button.h"
#include <QColorDialog>
#include <QPaintEvent>
#include <QPainter>

namespace crdc {
namespace airi {

ColorButton::ColorButton(const QColor &color, const std::function<void(QColor &color)> &callback)
    : color_(color), callback_(callback) {
  QObject::connect(this, &ColorButton::clicked, [&]() {
    auto color = QColorDialog::getColor(color_);
    if (color.isValid()) {
      color_ = color;
      repaint();
      callback_(color);
    }
  });
}

void ColorButton::paintEvent(QPaintEvent *event) {
  QPushButton::paintEvent(event);

  QPainter painter(this);
  painter.setBrush(QBrush(color_));
  painter.setPen(Qt::NoPen);
  auto rect = event->rect();
  constexpr int padding = 5;
  rect.adjust(padding, padding, -1 - padding, -1 - padding);
  painter.drawRect(rect);
}

}  // namespace airi
}  // namespace crdc
