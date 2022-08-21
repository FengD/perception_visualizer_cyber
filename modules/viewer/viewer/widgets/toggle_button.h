#pragma once

#include <QIcon>
#include <QPushButton>
#include <QString>
#include <functional>

namespace crdc {
namespace airi {

class ToggleButton : public QPushButton {
 public:
  ToggleButton(const QString &text, const bool is_checked,
               const std::function<void(bool)> &callback_toggled);
  ToggleButton(const QIcon &icon, const bool is_checked,
               const std::function<void(bool)> &callback_toggled);
};

}  // namespace airi
}  // namespace crdc
