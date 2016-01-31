// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_FORMS_BUTTON_CONTROL_H_
#define EVITA_DOM_FORMS_BUTTON_CONTROL_H_

#include "evita/dom/forms/form_control.h"

namespace dom {

namespace bindings {
class ButtonControlClass;
}

//////////////////////////////////////////////////////////////////////
//
// ButtonControl
//
class ButtonControl final
    : public v8_glue::Scriptable<ButtonControl, FormControl> {
  DECLARE_SCRIPTABLE_OBJECT(ButtonControl);

 public:
  ~ButtonControl() final;

  // Expose |text| for |view::FormWindow|.
  const base::string16& text() const { return text_; }

 private:
  friend class bindings::ButtonControlClass;

  // bindings
  explicit ButtonControl(const base::string16& text);

  void set_text(const base::string16& text);

  // FormControl
  std::unique_ptr<domapi::FormControl> Paint(
      const FormPaintInfo& paint_info) const final;

  base::string16 text_;

  DISALLOW_COPY_AND_ASSIGN(ButtonControl);
};

}  // namespace dom

#endif  // EVITA_DOM_FORMS_BUTTON_CONTROL_H_
