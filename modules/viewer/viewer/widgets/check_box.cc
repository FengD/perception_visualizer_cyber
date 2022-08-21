#include "viewer/widgets/check_box.h"

namespace crdc {
namespace airi {

CheckBox::CheckBox(const QString &text, const bool is_checked,
                   const std::function<void(bool)> &callback_toggled) {
  this->setText(text);
  this->setCheckable(true);
  this->setChecked(is_checked);
  QObject::connect(this, &QCheckBox::toggled, callback_toggled);
}

}  // namespace airi
}  // namespace crdc
