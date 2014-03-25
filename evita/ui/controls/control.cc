// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/ui/controls/control.h"

#include "evita/ui/controls/control_controller.h"

namespace ui {

//////////////////////////////////////////////////////////////////////
//
// Control
//
Control::Control(ControlController* controller)
    : controller_(controller), hover_(false) {
}

Control::~Control() {
  DCHECK(!controller_);
}

bool Control::focusable() const {
  return true;
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
  if (!hover_ || !focusable())
    return;
  DVLOG(0) << "End Hover " << this;
  hover_ = false;
  SchedulePaint();
}

void Control::OnMouseMoved(const MouseEvent& event) {
  DCHECK(controller_);
  controller_->OnMouseMoved(this, event);
  if (hover_ || !focusable())
    return;
  DVLOG(0) << "Start Hover " << this;
  hover_ = true;
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
