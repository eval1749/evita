// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/ui/controls/control.h"

#include "evita/ui/base/ime/text_input_client.h"
#include "evita/ui/controls/control_controller.h"
#include "evita/ui/events/event.h"

namespace ui {
//////////////////////////////////////////////////////////////////////
//
// Control::Style
//
bool Control::Style::operator==(const Style& other) const {
  return bgcolor == other.bgcolor && color == other.color &&
         font_family == other.font_family && font_size == other.font_size &&
         gray_text == other.gray_text && highlight == other.highlight &&
         hotlight == other.hotlight && shadow == other.shadow;
}

bool Control::Style::operator!=(const Style& other) const {
  return !operator==(other);
}

//////////////////////////////////////////////////////////////////////
//
// Control
//
Control::Control(ControlController* controller)
    : controller_(controller),
      state_(State::Normal),
      text_input_delegate_(nullptr) {}

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

// ui::EventTarget
void Control::OnKeyEvent(ui::KeyEvent* event) {
  DCHECK(controller_);
  if (disabled())
    return;
  controller_->OnKeyEvent(this, *event);
}

void Control::OnMouseEvent(ui::MouseEvent* event) {
  DCHECK(controller_);
  if (disabled())
    return;
  controller_->OnMouseEvent(this, *event);
  ui::EventTarget::OnMouseEvent(event);
  if (!focusable())
    return;
  if (event->event_type() == ui::EventType::MouseEntered) {
    if (state_ == State::Normal) {
      state_ = State::Hovered;
      DidChangeState();
    }
    return;
  }
  if (event->event_type() == ui::EventType::MouseExited) {
    if (state_ == State::Hovered) {
      state_ = State::Normal;
      DidChangeState();
    }
    return;
  }
}

// ui::Widget
void Control::DidKillFocus(ui::Widget* focused_widget) {
  Widget::DidKillFocus(focused_widget);
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
  Widget::DidRealize();
  controller_->DidRealize(this);
}

void Control::DidSetFocus(ui::Widget* last_focused_widget) {
  Widget::DidSetFocus(last_focused_widget);
  if (disabled())
    return;
  state_ = State::Highlight;
  if (is_realized()) {
    ui::TextInputClient::Get()->set_delegate(text_input_delegate_);
  }
  DidChangeState();
  controller_->DidSetFocus(this, last_focused_widget);
}

void Control::WillDestroyWidget() {
  Widget::WillDestroyWidget();
  auto const controller = controller_;
  controller_ = nullptr;
  controller->WillDestroyControl(this);
}

}  // namespace ui
