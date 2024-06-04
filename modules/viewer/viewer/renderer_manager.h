#pragma once

#include <QLayout>
#include <QWidget>
#include <QScrollArea>
#include <QResizeEvent>

namespace crdc {
namespace airi {

class RendererManager : public QWidget {
 public:
  RendererManager();

 public:
  void addWidget(QWidget *widget);

 protected:
  void resizeEvent(QResizeEvent *e) override;

 protected:
  QHBoxLayout *hbox_;
  QVBoxLayout *vbox_;
  QVBoxLayout *layout_;

  QWidget *container_;
  QScrollArea *scroll_area_;
};

}  // namespace airi
}  // namespace crdc
