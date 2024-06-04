#include "viewer/widgets/renderer_item.h"
#include "viewer/widgets/header_button.h"
#include "viewer/widgets/slide_toggle_button.h"

namespace crdc {
namespace airi {

RendererItem::RendererItem(const QString &text, const bool is_checked,
                           const std::function<void(bool)> &callback_toggled)
    : name_(text) {
  hbox_ = new QHBoxLayout();
  vbox_ = new QVBoxLayout();
  layout_ = new QVBoxLayout();
  container_ = new QWidget();
  container_->setVisible(false);
  container_->setLayout(vbox_);

  auto header_button =
      new HeaderButton(text, false, [&](bool is_checked) { container_->setVisible(is_checked); });

  auto slide_toggle_button = new SlideToggleButton(is_checked, callback_toggled);

  hbox_->addWidget(header_button);
  hbox_->addWidget(slide_toggle_button);
  layout_->addLayout(hbox_);
  layout_->addWidget(container_);
  this->setLayout(layout_);

  vbox_->setSpacing(0);
  vbox_->setContentsMargins(10, 0, 0, 5);
  layout_->setContentsMargins(0, 0, 0, 3);
}

QString RendererItem::name() const { return name_; }

void RendererItem::addWidget(QWidget *widget) { vbox_->addWidget(widget); }

void RendererItem::addLayout(QLayout *layout) { vbox_->addLayout(layout); }

void RendererItem::setUnfolded(bool is_unfolded) {
  ((HeaderButton *)(hbox_->itemAt(0)->widget()))->setChecked(is_unfolded);
}

void RendererItem::setChecked(bool is_checked) {
  ((HeaderButton *)(hbox_->itemAt(1)->widget()))->setChecked(is_checked);
}

}  // namespace airi
}  // namespace crdc
