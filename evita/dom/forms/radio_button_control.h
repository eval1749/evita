// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_dom_forms_radio_button_control_h)
#define INCLUDE_evita_dom_forms_radio_button_control_h

#include "evita/dom/forms/form_control.h"

namespace dom {

class RadioButtonControl
    : public v8_glue::Scriptable<RadioButtonControl, FormControl> {
  DECLARE_SCRIPTABLE_OBJECT(RadioButtonControl);

  private: bool checked_;

  public: RadioButtonControl(const base::string16& name,
                             FormResourceId control_id);
  public: virtual ~RadioButtonControl();

  public: const bool checked() const { return checked_; }
  public: void set_checked(bool checked);

  // EventTarget
  private: virtual bool DispatchEvent(Event* event) override;

  DISALLOW_COPY_AND_ASSIGN(RadioButtonControl);
};

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_forms_radio_button_control_h)
