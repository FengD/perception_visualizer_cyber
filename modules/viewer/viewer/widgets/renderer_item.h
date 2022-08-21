#pragma once

#include <QLayout>
#include <QString>
#include <QWidget>
#include <functional>

namespace crdc {
namespace airi {

class RendererItem : public QWidget {
 public:
  RendererItem(const QString &text, const bool is_checked,
               const std::function<void(bool)> &callback_toggled);

 public:
  QString name() const;
  void addWidget(QWidget *widget);
  void addLayout(QLayout *layout);
  void setUnfolded(bool is_unfolded);
  void setChecked(bool is_checked);

 protected:
  QString name_;
  QHBoxLayout *hbox_;
  QVBoxLayout *vbox_;
  QVBoxLayout *layout_;
  QWidget *container_;
};

}  // namespace airi
}  // namespace crdc
