// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/ui/controls/control.h"

#include "evita/ui/base/ime/text_input_client.h"
#include "evita/ui/controls/control_controller.h"

namespace ui {
//////////////////////////////////////////////////////////////////////
//
// Control::Style
//
bool Control::Style::operator==(const Style& other) const {
  return bgcolor == other.bgcolor && color == other.color &&
         font_family == other.font_family && font_size == other.font_size &&
         gray_text == other.gray_text && highlight == other.highlight &&
         hotlight == other.hotlight &&
         shadow == other.shadow;
}

bool Control::Style::operator!=(const Style& other) const {
  return !operator==(other);
}

//////////////////////////////////////////////////////////////////////
//
// Control
//
Control::Control(ControlController* controller)
    : controller_(controller), state_(State::Normal),
      text_input_delegate_(nullptr) {
}

Control::~Control() {
  DCHECK(!controller_);
}

bool Control::focusable() const {
  return !disabled();
}

void Control::set_disabled(bool new_disabled) {
  if (disabled() == new_disabled)
    return;
  state_ = new_disabled ? State::Disabled : State::Normal;
  DidChangeState();
}

void Control::set_text_input_delegate(TextInputDelegate* delegate) {
  DCHECK(!text_input_delegate_);
  text_input_delegate_ = delegate;
}

void Control::DidChangeState() {
  if (!is_realized())
    return;
  SchedulePaint();
}

// ui::WIdget
void Control::DidKillFocus(ui::Widget* focused_widget) {
  AnimatableWindow::DidKillFocus(focused_widget);
  if (disabled()) {
    return;
  }
  state_ = State::Normal;
  if (text_input_delegate_) {
    ui::TextInputClient::Get()->CommitComposition(text_input_delegate_);
    ui::TextInputClient::Get()->CancelComposition(text_input_delegate_);
    ui::TextInputClient::Get()->set_delegate(nullptr);
  }
  DidChangeState();
  controller_->DidKillFocus(this, focused_widget);
}

void Control::DidRealize() {
  AnimatableWindow::DidRealize();
  controller_->DidRealize(this);
}

void Control::DidSetFocus(ui::Widget* last_focused_widget) {
  AnimatableWindow::DidSetFocus(last_focused_widget);
  if (disabled())
    return;
  state_ = State::Highlight;
  if (is_realized()) {
    ui::TextInputClient::Get()->set_delegate(text_input_delegate_);
  }
  DidChangeState();
  controller_->DidSetFocus(this, last_focused_widget);
}

void Control::OnKeyPressed(const KeyboardEvent& event) {
  DCHECK(controller_);
  if (disabled())
    return;
  controller_->OnKeyPressed(this, event);
}

void Control::OnKeyReleased(const KeyboardEvent& event) {
  DCHECK(controller_);
  if (disabled())
    return;
  controller_->OnKeyReleased(this, event);
}

void Control::OnMouseExited(const MouseEvent& event) {
  DCHECK(controller_);
  if (disabled())
    return;
  controller_->OnMouseExited(this, event);
  if (state_ != State::Hover || !focusable())
    return;
  state_ = State::Normal;
  DidChangeState();
}

void Control::OnMouseMoved(const MouseEvent& event) {
  DCHECK(controller_);
  if (disabled())
    return;
  controller_->OnMouseMoved(this, event);
  if (state_ != State::Normal || !focusable())
    return;
  state_ = State::Hover;
  DidChangeState();
}

void Control::OnMousePressed(const MouseEvent& event) {
  DCHECK(controller_);
  if (disabled())
    return;
  controller_->OnMousePressed(this, event);
}

void Control::OnMouseReleased(const MouseEvent& event) {
  DCHECK(controller_);
  if (disabled())
    return;
  controller_->OnMouseReleased(this, event);
}

void Control::OnMouseWheel(const MouseWheelEvent& event) {
  DCHECK(controller_);
  if (disabled())
    return;
  controller_->OnMouseWheel(this, event);
}

void Control::WillDestroyWidget() {
  AnimatableWindow::WillDestroyWidget();
  auto const controller = controller_;
  controller_ = nullptr;
  controller->WillDestroyControl(this);
}

}  // namespace ui
