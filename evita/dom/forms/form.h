// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_dom_forms_form_h)
#define INCLUDE_evita_dom_forms_form_h

#include "evita/dom/events/event_target.h"
#include "evita/dom/public/dialog_box_id.h"

#include <vector>

namespace dom {

class FormControl;

class Form : public v8_glue::Scriptable<Form, EventTarget> {
  DECLARE_SCRIPTABLE_OBJECT(Form);

  private: std::vector<FormControl*> controls_;

  public: Form();
  public: virtual ~Form();

  public: const std::vector<FormControl*>& controls() const {
    return controls_;
  }
  public: domapi::DialogBoxId dialog_box_id() const {
    return event_target_id();
  }

  public: void AddFormControl(FormControl* control);
  public: void Realize();
  public: void Show();

  DISALLOW_COPY_AND_ASSIGN(Form);
};

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_forms_form_h)
