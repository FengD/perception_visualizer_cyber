#include "viewer/widgets/toggle_button.h"

namespace crdc {
namespace airi {

ToggleButton::ToggleButton(const QString &text, const bool is_checked,
                           const std::function<void(bool)> &callback_toggled) {
  this->setText(text);
  this->setCheckable(true);
  this->setChecked(is_checked);
  QObject::connect(this, &QPushButton::toggled, callback_toggled);
}

ToggleButton::ToggleButton(const QIcon &icon, const bool is_checked,
                           const std::function<void(bool)> &callback_toggled) {
  this->setIcon(icon);
  this->setCheckable(true);
  this->setChecked(is_checked);
  QObject::connect(this, &QPushButton::toggled, callback_toggled);
}

}  // namespace airi
}  // namespace crdc
