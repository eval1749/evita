// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_dom_forms_text_control_h)
#define INCLUDE_evita_dom_forms_text_control_h

#include "evita/dom/forms/form_control.h"

namespace dom {

class TextFieldControl
    : public v8_glue::Scriptable<TextFieldControl, FormControl> {
  DECLARE_SCRIPTABLE_OBJECT(TextFieldControl);

  private: base::string16 value_;

  public: TextFieldControl(FormResourceId control_id);
  public: virtual ~TextFieldControl();

  public: const base::string16& value() const { return value_; }
  public: void set_value(const base::string16& value);

  // EventTarget
  private: virtual bool DispatchEvent(Event* event) override;

  DISALLOW_COPY_AND_ASSIGN(TextFieldControl);
};

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_forms_text_control_h)
