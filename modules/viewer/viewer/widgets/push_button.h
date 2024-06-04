#pragma once

#include <QIcon>
#include <QPushButton>
#include <QString>
#include <functional>

namespace crdc {
namespace airi {

class PushButton : public QPushButton {
 public:
  PushButton(const QString &text, const std::function<void()> &callback_clicked);
  PushButton(const QIcon &icon, const std::function<void()> &callback_clicked);
};

}  // namespace airi
}  // namespace crdc
