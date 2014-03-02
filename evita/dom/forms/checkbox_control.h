// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_dom_forms_checkbox_control_h)
#define INCLUDE_evita_dom_forms_checkbox_control_h

#include "evita/dom/forms/form_control.h"

namespace dom {

class CheckboxControl
    : public v8_glue::Scriptable<CheckboxControl, FormControl> {
  DECLARE_SCRIPTABLE_OBJECT(CheckboxControl);

  private: bool checked_;

  public: CheckboxControl(FormResourceId control_id);
  public: virtual ~CheckboxControl();

  public: const bool checked() const { return checked_; }
  public: void set_checked(bool checked);

  // EventTarget
  private: virtual bool DispatchEvent(Event* event) override;

  DISALLOW_COPY_AND_ASSIGN(CheckboxControl);
};

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_forms_checkbox_control_h)
