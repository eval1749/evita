// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/views/forms/form_control_controller.h"

#include <unordered_map>

#include "common/memory/singleton.h"
#include "evita/dom/public/view_event.h"
#include "evita/ui/controls/control.h"

namespace views {

namespace {

//////////////////////////////////////////////////////////////////////
//
// ControlSet
//
class ControlSet : public common::Singleton<ControlSet> {
  DECLARE_SINGLETON_CLASS(ControlSet);

  private: std::unordered_map<ui::Control*, domapi::EventTargetId> map_;

  private: ControlSet();
  public: ~ControlSet();

  public: domapi::EventTargetId ControlSet::MaybeControlId(
      ui::Widget* widget) const;
  public: void Register(ui::Control* control,
                        domapi::EventTargetId target_id);
  public: void Unregister(ui::Control* control);

  DISALLOW_COPY_AND_ASSIGN(ControlSet);
};

ControlSet::ControlSet() {
}

ControlSet::~ControlSet() {
}

domapi::EventTargetId ControlSet::MaybeControlId(ui::Widget* widget) const {
  if (!widget)
    return domapi::kInvalidEventTargetId;
  auto const control = widget->as<ui::Control>();
  if (!control)
    return domapi::kInvalidEventTargetId;
  auto const it = map_.find(control);
  return it == map_.end() ? domapi::kInvalidEventTargetId : it->second;
}

void ControlSet::Register(ui::Control* control,
                        domapi::EventTargetId target_id) {
  map_[control] = target_id;
}

void ControlSet::Unregister(ui::Control* control) {
  auto it = map_.find(control);
  if (it == map_.end())
    return;
  map_.erase(it);
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// FormControlController
//
FormControlController::FormControlController(
    domapi::EventTargetId event_target_id)
    : EventSource(event_target_id) {
}

FormControlController::~FormControlController() {
}

// ui::ControlController
void FormControlController::DidKillFocus(ui::Control*,
                                         ui::Widget* related_widget) {
  DispatchFocusEvent(domapi::EventType::Blur,
                     ControlSet::instance()->MaybeControlId(related_widget));
}

void FormControlController::DidRealize(ui::Control* control) {
  ControlSet::instance()->Register(control, event_target_id());
}

void FormControlController::DidSetFocus(ui::Control*,
                                        ui::Widget* related_widget) {
  DispatchFocusEvent(domapi::EventType::Focus,
                     ControlSet::instance()->MaybeControlId(related_widget));
}

void FormControlController::OnKeyPressed(ui::Control*,
                                         const ui::KeyboardEvent& event) {
  DispatchKeyboardEvent(event);
}

void FormControlController::OnKeyReleased(ui::Control*,
                                          const ui::KeyboardEvent& event) {
  DispatchKeyboardEvent(event);
}

void FormControlController::OnMouseMoved(ui::Control*,
                                         const ui::MouseEvent& event) {
  DispatchMouseEvent(event);
}

void FormControlController::OnMousePressed(ui::Control*,
                                           const ui::MouseEvent& event) {
  DispatchMouseEvent(event);
}

void FormControlController::OnMouseReleased(ui::Control*,
                                            const ui::MouseEvent& event) {
  DispatchMouseEvent(event);
}

void FormControlController::OnMouseWheel(ui::Control*,
                                         const ui::MouseWheelEvent& event) {
  DispatchWheelEvent(event);
}

void FormControlController::WillDestroyControl(ui::Control* control) {
  ControlSet::instance()->Unregister(control);
  delete this;
}

}  // namespace views
