// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/forms/form.h"

#include "evita/dom/bindings/exception_state.h"
#include "evita/dom/forms/form_control.h"
#include "evita/dom/forms/form_observer.h"

namespace dom {

//////////////////////////////////////////////////////////////////////
//
// Form
//
Form::Form() {}
Form::~Form() {}

std::vector<FormControl*> Form::controls() const {
  return controls_;
}

void Form::set_focus_control(v8_glue::Nullable<FormControl> new_focus_control) {
  if (focus_control_ == new_focus_control)
    return;
  focus_control_ = new_focus_control;
  FOR_EACH_OBSERVER(FormObserver, observers_, DidChangeForm());
}

void Form::set_height(int new_height) {
  if (bounds_.height() == new_height)
    return;
  bounds_ = IntRect(bounds_.origin(), IntSize(width(), new_height));
  FOR_EACH_OBSERVER(FormObserver, observers_, DidChangeForm());
}

void Form::set_title(const base::string16& new_title) {
  if (title_ == new_title)
    return;
  title_ = new_title;
  FOR_EACH_OBSERVER(FormObserver, observers_, DidChangeForm());
}

void Form::set_width(int new_width) {
  if (bounds_.width() == new_width)
    return;
  bounds_ = IntRect(bounds_.origin(), IntSize(new_width, height()));
  FOR_EACH_OBSERVER(FormObserver, observers_, DidChangeForm());
}

void Form::AddFormControl(FormControl* control,
                          ExceptionState* exception_state) {
  if (control->form_) {
    exception_state->ThrowError("Specified control is already added to form.");
    return;
  }
  controls_.push_back(control);
  control->form_ = this;
  FOR_EACH_OBSERVER(FormObserver, observers_, DidChangeForm());
}

void Form::AddObserver(FormObserver* observer) const {
  observers_.AddObserver(observer);
}

void Form::DidChangeFormControl(FormControl* control) {
  DCHECK(control);
  FOR_EACH_OBSERVER(FormObserver, observers_, DidChangeForm());
}

void Form::DidKillFocusFromFormControl(FormControl* control) {
  if (focus_control_ != control)
    return;
  focus_control_ = nullptr;
}

void Form::DidSetFocusToFormControl(FormControl* control) {
  focus_control_ = control;
}

void Form::RemoveObserver(FormObserver* observer) const {
  observers_.RemoveObserver(observer);
}

}  // namespace dom
