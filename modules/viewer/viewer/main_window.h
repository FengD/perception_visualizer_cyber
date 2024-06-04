#pragma once

#include <QKeyEvent>
#include <QWidget>

namespace crdc {
namespace airi {

class MainWindow : public QWidget {
 public:
  MainWindow();

 protected:
  void keyPressEvent(QKeyEvent *e) override;
};

}  // namespace airi
}  // namespace crdc
