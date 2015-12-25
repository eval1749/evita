// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/ui/controls/button.h"

#include "evita/gfx/brush.h"
#include "evita/gfx/canvas.h"
#include "evita/ui/events/event.h"

namespace ui {

//////////////////////////////////////////////////////////////////////
//
// Button
//
Button::Button(ButtonListener* listener)
    : canvas_bitmap_id_(0),
      dirty_(true),
      listener_(listener),
      state_(State::Normal) {}

Button::~Button() {}

bool Button::IsDirty(const gfx::Canvas* canvas) const {
  return dirty_ || canvas_bitmap_id_ != canvas->bitmap_id();
}

void Button::MarkDirty() {
  if (dirty_ || !visible())
    return;
  SchedulePaint();
  dirty_ = true;
}

void Button::PaintButton(gfx::Canvas*) {}

void Button::SetState(State new_state) {
  if (state_ == new_state)
    return;
  state_ = new_state;
  MarkDirty();
}

// ui::Widget
void Button::DidChangeBounds() {
  ui::Widget::DidChangeBounds();
  MarkDirty();
}

void Button::DidShow() {
  ui::Widget::DidShow();
  MarkDirty();
}

void Button::OnDraw(gfx::Canvas* canvas) {
  if (!IsDirty(canvas))
    return;
  canvas_bitmap_id_ = canvas->bitmap_id();
  dirty_ = false;
  PaintButton(canvas);
}

void Button::OnMouseEntered(const ui::MouseEvent&) {
  SetState(State::Hovered);
}

void Button::OnMouseExited(const ui::MouseEvent&) {
  SetState(State::Normal);
}

void Button::OnMousePressed(const ui::MouseEvent& event) {
  if (event.button() != ui::MouseButton::Left)
    return;
  SetState(State::Pressed);
  listener_->DidPressButton(this, event);
}

void Button::OnMouseReleased(const ui::MouseEvent& event) {
  if (event.button() != ui::MouseButton::Left)
    return;
  SetState(State::Hovered);
}

//////////////////////////////////////////////////////////////////////
//
// ButtonListener
//
ButtonListener::ButtonListener() {}

ButtonListener::~ButtonListener() {}

}  // namespace ui
