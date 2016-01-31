// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/forms/form_control.h"

#include "evita/dom/events/event.h"
#include "evita/bindings/v8_glue_EventInit.h"
#include "evita/dom/forms/form.h"
#include "evita/dom/forms/form_paint_info.h"

namespace dom {

//////////////////////////////////////////////////////////////////////
//
// FormControl
//
FormControl::FormControl(const base::string16& name)
    : disabled_(false), handling_form_event_(false), name_(name) {}

FormControl::FormControl() : FormControl(base::string16()) {}

FormControl::~FormControl() {}

void FormControl::set_client_height(float new_height) {
  if (bounds_.height() == new_height)
    return;
  bounds_ = domapi::IntRect(bounds_.origin(),
                            domapi::IntSize(bounds_.width(), new_height));
  NotifyControlChange();
}

void FormControl::set_client_left(float new_left) {
  if (bounds_.x() == new_left)
    return;
  bounds_ =
      domapi::IntRect(domapi::IntPoint(new_left, bounds_.y()), bounds_.size());
  NotifyControlChange();
}

void FormControl::set_client_top(float new_top) {
  if (bounds_.y() == new_top)
    return;
  bounds_ =
      domapi::IntRect(domapi::IntPoint(bounds_.x(), new_top), bounds_.size());
  NotifyControlChange();
}

void FormControl::set_client_width(float new_width) {
  if (bounds_.width() == new_width)
    return;
  bounds_ = domapi::IntRect(bounds_.origin(),
                            domapi::IntSize(new_width, bounds_.height()));
  NotifyControlChange();
}

void FormControl::set_disabled(bool new_disabled) {
  if (disabled_ == new_disabled)
    return;
  disabled_ = new_disabled;
  NotifyControlChange();
}

domapi::FormControl::State FormControl::ComputeState(
    const FormPaintInfo& paint_info) const {
  domapi::FormControl::State state;
  if (disabled())
    state.set_disabled();
  if (!paint_info.is_interactive())
    return state;
  if (form()->focus_control() == this)
    state.set_focused();
  if (paint_info.hovered_control() == this)
    state.set_hovered();
  return state;
}

void FormControl::NotifyControlChange() {
  if (!form_)
    return;
  form_->DidChangeFormControl(this);
}

// dom::EventTarget
EventTarget::EventPath FormControl::BuildEventPath() const {
  return form_ ? EventPath{form_} : EventPath();
}

}  // namespace dom
