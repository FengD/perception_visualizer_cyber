#include "viewer/widgets/slide_toggle_button.h"
#include <QPainter>

namespace crdc {
namespace airi {

SlideToggleButton::SlideToggleButton(const bool is_checked,
                                     const std::function<void(bool)> &callback_toggled) {
  this->setCheckable(true);
  this->setChecked(is_checked);
  QObject::connect(this, &QPushButton::toggled, callback_toggled);

  setMaximumHeight(22);
  setMaximumWidth(38);
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
}

void SlideToggleButton::paintEvent(QPaintEvent * /*e*/) {
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);
  painter.setPen(Qt::PenStyle(Qt::NoPen));
  painter.setBrush(isChecked() ? QColor(74, 211, 96) : QColor(0, 0, 0, 32));
  painter.drawRoundedRect(0, 1, width(), height() - 2, (height() - 2) / 2, (height() - 2) / 2);

  painter.setPen(QPen(QBrush(QColor(0, 0, 0, 32)), 1.0));
  painter.setBrush(QColor(255, 255, 255));
  painter.drawEllipse(isChecked() ? width() - height() + 2 : 1, 2, height() - 4, height() - 4);
}

}  // namespace airi
}  // namespace crdc
