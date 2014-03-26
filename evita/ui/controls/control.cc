// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/ui/controls/control.h"

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
    : controller_(controller), state_(State::Normal) {
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
  SchedulePaint();
}

void Control::OnKeyPressed(const KeyboardEvent& event) {
  DCHECK(controller_);
  controller_->OnKeyPressed(this, event);
}

void Control::OnKeyReleased(const KeyboardEvent& event) {
  DCHECK(controller_);
  controller_->OnKeyReleased(this, event);
}

void Control::OnMouseExited(const MouseEvent& event) {
  controller_->OnMouseExited(this, event);
  if (state_ != State::Hover || !focusable())
    return;
  state_ = State::Normal;
  SchedulePaint();
}

void Control::OnMouseMoved(const MouseEvent& event) {
  DCHECK(controller_);
  controller_->OnMouseMoved(this, event);
  if (state_ == State::Normal || !focusable())
    return;
  state_ = State::Hover;
  SchedulePaint();
}

void Control::OnMousePressed(const MouseEvent& event) {
  DCHECK(controller_);
  controller_->OnMousePressed(this, event);
}

void Control::OnMouseReleased(const MouseEvent& event) {
  DCHECK(controller_);
  controller_->OnMouseReleased(this, event);
}

void Control::OnMouseWheel(const MouseWheelEvent& event) {
  DCHECK(controller_);
  controller_->OnMouseWheel(this, event);
}

void Control::WillDestroyWidget() {
  auto const controller = controller_;
  controller_ = nullptr;
  controller->WillDestroyControl(this);
}

}  // namespace ui
