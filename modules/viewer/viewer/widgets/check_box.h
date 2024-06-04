#pragma once

#include <QCheckBox>
#include <QString>
#include <functional>

namespace crdc {
namespace airi {

class CheckBox : public QCheckBox {
 public:
  CheckBox(const QString &text, const bool is_checked,
           const std::function<void(bool)> &callback_toggled);
};

}  // namespace airi
}  // namespace crdc
