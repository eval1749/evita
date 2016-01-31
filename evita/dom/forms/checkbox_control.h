// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_FORMS_CHECKBOX_CONTROL_H_
#define EVITA_DOM_FORMS_CHECKBOX_CONTROL_H_

#include "evita/dom/forms/form_control.h"

namespace dom {

namespace bindings {
class CheckboxControlClass;
}

class ExceptionState;

//////////////////////////////////////////////////////////////////////
//
// CheckboxControl
//
class CheckboxControl final
    : public v8_glue::Scriptable<CheckboxControl, FormControl> {
  DECLARE_SCRIPTABLE_OBJECT(CheckboxControl);

 public:
  ~CheckboxControl() final;

  bool checked() const { return checked_; }

 private:
  friend class bindings::CheckboxControlClass;

  CheckboxControl();

  // binding
  void set_checked(bool checked);

  // FormControl
  std::unique_ptr<domapi::FormControl> Paint(
      const FormPaintInfo& paint_info) const final;

  bool checked_ = false;

  DISALLOW_COPY_AND_ASSIGN(CheckboxControl);
};

}  // namespace dom

#endif  // EVITA_DOM_FORMS_CHECKBOX_CONTROL_H_
