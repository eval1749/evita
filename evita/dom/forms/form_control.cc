// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/forms/form_control.h"

#include "evita/dom/events/event.h"
#include "evita/bindings/EventInit.h"
#include "evita/dom/forms/form.h"

namespace dom {

//////////////////////////////////////////////////////////////////////
//
// FormControl::HandlingFormEventScope
//
FormControl::HandlingFormEventScope::HandlingFormEventScope(
    FormControl* control) : control_(control) {
  DCHECK(!control_->handling_form_event_);
  control_->handling_form_event_ = true;
}

FormControl::HandlingFormEventScope::~HandlingFormEventScope() {
  DCHECK(control_->handling_form_event_);
  control_->handling_form_event_ = false;
}

//////////////////////////////////////////////////////////////////////
//
// FormControl
//
FormControl::FormControl(const base::string16& name)
    : disabled_(false), handling_form_event_(false), name_(name) {
}

FormControl::FormControl() : FormControl(base::string16()) {
}

FormControl::~FormControl() {
}

void FormControl::set_client_height(float new_height) {
  if (bounds_.height() == new_height)
    return;
  bounds_.bottom = bounds_.top + new_height;
  NotifyControlChange();
}

void FormControl::set_client_left(float new_left) {
  if (bounds_.left == new_left)
    return;
  auto const width = bounds_.width();
  bounds_.left = new_left;
  bounds_.right = new_left + width;
  NotifyControlChange();
}

void FormControl::set_client_top(float new_top) {
  if (bounds_.top == new_top)
    return;
  auto const height = bounds_.height();
  bounds_.top = new_top;
  bounds_.bottom = new_top + height;
  NotifyControlChange();
}

void FormControl::set_client_width(float new_width) {
  if (bounds_.width() == new_width)
    return;
  bounds_.right = bounds_.left + new_width;
  NotifyControlChange();
}

void FormControl::set_disabled(bool new_disabled) {
  if (disabled_ == new_disabled)
    return;
  disabled_ = new_disabled;
  NotifyControlChange();
}

void FormControl::DidSetFocus() {
  if (!form_)
    return;
  form_->DidSetFocusToFormControl(this);
}

void FormControl::DispatchChangeEvent() {
  if (!handling_form_event_ && form_)
    NotifyControlChange();
  EventInit init_dict;
  init_dict.set_bubbles(true);
  init_dict.set_cancelable(false);
  DispatchEvent(new Event(L"change", init_dict));
}

void FormControl::NotifyControlChange() {
  if (!form_)
    return;
  form_->DidChangeFormControl(this);
}

// dom::EventTarget
EventTarget::EventPath FormControl::BuildEventPath() const {
  return form_ ? EventPath {form_} : EventPath();
}

}  // namespace dom
