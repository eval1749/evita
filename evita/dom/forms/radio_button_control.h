// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_FORMS_RADIO_BUTTON_CONTROL_H_
#define EVITA_DOM_FORMS_RADIO_BUTTON_CONTROL_H_

#include "evita/dom/forms/form_control.h"

namespace dom {

namespace bindings {
class RadioButtonControlClass;
}

class RadioButtonControl final
    : public v8_glue::Scriptable<RadioButtonControl, FormControl> {
  DECLARE_SCRIPTABLE_OBJECT(RadioButtonControl);

 public:
  virtual ~RadioButtonControl();

  bool checked() const { return checked_; }

 private:
  friend class bindings::RadioButtonControlClass;

  explicit RadioButtonControl(const base::string16& name);

  void set_checked(bool new_checked);

  // EventTarget
  bool DispatchEvent(Event* event) final;

  bool checked_;

  DISALLOW_COPY_AND_ASSIGN(RadioButtonControl);
};

}  // namespace dom

#endif  // EVITA_DOM_FORMS_RADIO_BUTTON_CONTROL_H_
