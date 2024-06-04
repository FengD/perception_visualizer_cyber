#pragma once

#include <QPushButton>
#include <QString>
#include <functional>

namespace crdc {
namespace airi {

class SlideToggleButton : public QPushButton {
 public:
  SlideToggleButton(const bool is_checked, const std::function<void(bool)> &callback_toggled);

 protected:
  virtual void paintEvent(QPaintEvent *e) override;
};

}  // namespace airi
}  // namespace crdc
