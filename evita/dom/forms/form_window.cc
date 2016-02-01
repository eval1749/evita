// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/forms/form_window.h"

#include "base/trace_event/trace_event.h"
#include "evita/bindings/v8_glue_FocusEventInit.h"
#include "evita/bindings/v8_glue_FormWindowInit.h"
#include "evita/dom/events/focus_event.h"
#include "evita/dom/events/keyboard_event.h"
#include "evita/dom/events/mouse_event.h"
#include "evita/dom/forms/form.h"
#include "evita/dom/forms/form_control.h"
#include "evita/dom/forms/form_paint_info_builder.h"
#include "evita/dom/public/caret_shape.h"
#include "evita/dom/public/form.h"
#include "evita/dom/public/form_controls.h"
#include "evita/dom/public/view_delegate.h"
#include "evita/dom/public/view_event.h"
#include "evita/dom/scheduler.h"
#include "evita/dom/script_host.h"
#include "evita/dom/timing/animation_frame_callback.h"

namespace dom {

using CaretShape = domapi::CaretShape;

//////////////////////////////////////////////////////////////////////
//
// FormWindow
//
FormWindow::FormWindow(ScriptHost* script_host,
                       Form* form,
                       const FormWindowInit& init)
    : Scriptable(script_host), form_(form) {
  const auto owner_id =
      init.owner() ? init.owner()->window_id() : domapi::kInvalidWindowId;
  script_host->view_delegate()->CreateFormWindow(window_id(), owner_id,
                                                 form->bounds(), form->title());
  form_->AddObserver(this);
}

FormWindow::FormWindow(ScriptHost* script_host, Form* form)
    : FormWindow(script_host, form, FormWindowInit()) {}

FormWindow::~FormWindow() {
  form_->RemoveObserver(this);
}

void FormWindow::DidBeginAnimationFrame(const base::TimeTicks& now) {
  DCHECK(is_waiting_animation_frame_);
  is_waiting_animation_frame_ = false;
  if (!visible())
    return;
  TRACE_EVENT_WITH_FLOW0("view", "FormWindow::DidBeginAnimationFrame",
                         form_->event_target_id(), TRACE_EVENT_FLAG_FLOW_OUT);
  const auto is_interactive = Window::GetFocusWindow() == this;
  FormPaintInfo::Builder paint_info;
  // TODO(eval1749): We should support caret blinking.
  paint_info.set_caret_shape(is_interactive ? CaretShape::Bar
                                            : CaretShape::None);
  paint_info.set_hovered_control(hovered_control_);
  paint_info.set_interactive(is_interactive);
  auto form_model = form_->Paint(paint_info.Build());
  script_host()->view_delegate()->PaintForm(window_id(), std::move(form_model));
}

void FormWindow::RequestAnimationFrame() {
  if (is_waiting_animation_frame_)
    return;
  is_waiting_animation_frame_ = true;
  auto callback = std::make_unique<AnimationFrameCallback>(
      FROM_HERE,
      base::Bind(&FormWindow::DidBeginAnimationFrame, base::Unretained(this)));
  script_host()->scheduler()->RequestAnimationFrame(std::move(callback));
}

void FormWindow::UpdateHoveredControl(FormControl* control) {
  if (hovered_control_ == control)
    return;
  hovered_control_ = control;
  RequestAnimationFrame();
}

// FormObserver
void FormWindow::DidChangeForm() {
  RequestAnimationFrame();
}

// ViewEventTarget
std::pair<EventTarget*, KeyboardEvent*> FormWindow::TranslateKeyboardEvent(
    const domapi::KeyboardEvent& view_event) {
  const auto& keyboard_event = new KeyboardEvent(view_event);
  if (form_->focus_control())
    return std::make_pair(form_->focus_control(), keyboard_event);
  return std::make_pair(form_.get(), keyboard_event);
}

std::pair<EventTarget*, MouseEvent*> FormWindow::TranslateMouseEvent(
    const domapi::MouseEvent& event) {
  // TODO(eval1749): We should support mouse capture for |FormControl|.
  for (const auto& control : form_->controls()) {
    const auto& control_point =
        domapi::IntPoint(event.client_x, event.client_y) -
        domapi::IntSize(control->bounds().x(), control->bounds().y());
    if (!domapi::IntRect(control->bounds().size()).Contains(control_point))
      continue;
    if (event.event_type == domapi::EventType::MouseMove)
      UpdateHoveredControl(control);
    auto control_event = event;
    control_event.client_x = control_point.x();
    control_event.client_y = control_point.y();
    return std::make_pair(control, new MouseEvent(control_event));
  }
  if (event.event_type == domapi::EventType::MouseMove)
    UpdateHoveredControl(nullptr);
  return std::make_pair(form_.get(), new MouseEvent(event));
}

// Window
void FormWindow::DidKillFocus() {
  Window::DidKillFocus();
  RequestAnimationFrame();
}

void FormWindow::DidSetFocus() {
  Window::DidSetFocus();
  RequestAnimationFrame();
}

void FormWindow::DidShowWindow() {
  Window::DidShowWindow();
  RequestAnimationFrame();
}

}  // namespace dom
