// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_dom_forms_form_control_h)
#define INCLUDE_evita_dom_forms_form_control_h

#include "evita/dom/events/event_target.h"

#include "base/strings/string16.h"

namespace dom {

typedef int FormResourceId;
const FormResourceId kInvalidFormResourceId = -1;

class FormControl : public v8_glue::Scriptable<FormControl, EventTarget> {
  DECLARE_SCRIPTABLE_OBJECT(FormControl);

  private: FormResourceId control_id_;

  protected: explicit FormControl(int control_id);
  protected: FormControl();
  public: virtual ~FormControl();

  public: FormResourceId control_id() const { return control_id_; }

  DISALLOW_COPY_AND_ASSIGN(FormControl);
};

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_forms_form_control_h)
