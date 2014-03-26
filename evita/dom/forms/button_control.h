// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_dom_forms_button_control_h)
#define INCLUDE_evita_dom_forms_button_control_h

#include "evita/dom/forms/form_control.h"

namespace dom {

class ButtonControl
    : public v8_glue::Scriptable<ButtonControl, FormControl> {
  DECLARE_SCRIPTABLE_OBJECT(ButtonControl);

  private: base::string16 text_;

  public: explicit ButtonControl(FormResourceId control_id);
  public: virtual ~ButtonControl();

  public: const base::string16& text() const { return text_; }
  public: void set_text(const base::string16& text);

  DISALLOW_COPY_AND_ASSIGN(ButtonControl);
};

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_forms_button_control_h)
