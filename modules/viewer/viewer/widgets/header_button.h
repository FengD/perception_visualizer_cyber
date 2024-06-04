#pragma once

#include <QPushButton>
#include <QString>
#include <functional>

namespace crdc {
namespace airi {

class HeaderButton : public QPushButton {
 public:
  HeaderButton(const QString &text, const bool is_checked,
               const std::function<void(bool)> &callback_toggled);

 protected:
  virtual void paintEvent(QPaintEvent *e) override;

 protected:
  QString text_;
};

}  // namespace airi
}  // namespace crdc
