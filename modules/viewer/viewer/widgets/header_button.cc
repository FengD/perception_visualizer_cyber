#include "viewer/widgets/header_button.h"
#include <QPainter>

namespace crdc {
namespace airi {

HeaderButton::HeaderButton(const QString &text, const bool is_checked,
                           const std::function<void(bool)> &callback_toggled) {
  text_ = text;
  this->setCheckable(true);
  this->setChecked(is_checked);
  QObject::connect(this, &QPushButton::toggled, callback_toggled);

  setMaximumHeight(22);
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
}

void HeaderButton::paintEvent(QPaintEvent * /*e*/) {
  QPainter painter(this);
  painter.setPen(Qt::PenStyle(Qt::NoPen));
  painter.setBrush(isChecked() ? QColor(0, 76, 153, 96) : QColor(0, 0, 0, 32));
  painter.drawRoundedRect(0, 1, width(), height() - 2, (height() - 2) / 4, (height() - 2) / 4);

  QFont font = painter.font();
  font.setPointSize(height() / 2);
  painter.setFont(font);
  painter.setPen(QColor(0, 0, 0, 192));
  painter.drawText(rect(), Qt::AlignLeft | Qt::AlignVCenter,
                   isChecked() ? "  ▼ " + text_ : "  ▶ " + text_);
}

}  // namespace airi
}  // namespace crdc
