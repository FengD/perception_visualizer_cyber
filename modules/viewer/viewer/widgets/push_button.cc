#include "viewer/widgets/push_button.h"

namespace crdc {
namespace airi {

PushButton::PushButton(const QString &text, const std::function<void()> &callback_clicked) {
  this->setText(text);
  QObject::connect(this, &QPushButton::clicked, callback_clicked);
}

PushButton::PushButton(const QIcon &icon, const std::function<void()> &callback_clicked) {
  this->setIcon(icon);
  QObject::connect(this, &QPushButton::clicked, callback_clicked);
}

}  // namespace airi
}  // namespace crdc
