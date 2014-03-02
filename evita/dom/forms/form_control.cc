// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/forms/form_control.h"

#include "evita/dom/events/event.h"
#include "evita/dom/events/event_init.h"
#include "evita/dom/forms/form.h"
#include "evita/dom/script_controller.h"
#include "evita/dom/view_delegate.h"
#include "evita/v8_glue/converter.h"
#include "evita/v8_glue/wrapper_info.h"

namespace dom {

namespace {
//////////////////////////////////////////////////////////////////////
//
// FormControlClass
//
class FormControlClass :
    public v8_glue::DerivedWrapperInfo<FormControl, EventTarget> {

  public: FormControlClass(const char* name)
      : BaseClass(name) {
  }
  public: ~FormControlClass() = default;

  private: virtual void SetupInstanceTemplate(
      ObjectTemplateBuilder& builder) override {
    builder
        .SetProperty("controlId", &FormControl::control_id)
        .SetProperty("form", &FormControl::form)
        .SetProperty("name", &FormControl::name);
  }

  DISALLOW_COPY_AND_ASSIGN(FormControlClass);
};
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// FormControl::HandlingFormEventScope
//
FormControl::HandlingFormEventScope::HandlingFormEventScope(
    FormControl* control) : control_(control) {
  DCHECK(!control_->handling_form_event_);
  control_->handling_form_event_ = true;
}

FormControl::HandlingFormEventScope::~HandlingFormEventScope() {
  DCHECK(control_->handling_form_event_);
  control_->handling_form_event_ = false;
}

//////////////////////////////////////////////////////////////////////
//
// FormControl
//
DEFINE_SCRIPTABLE_OBJECT(FormControl, FormControlClass);

FormControl::FormControl(FormResourceId control_id, const base::string16& name)
    : control_id_(control_id), handling_form_event_(false), name_(name) {
}

FormControl::FormControl(FormResourceId control_id)
    : FormControl(control_id, base::string16()) {
}

FormControl::FormControl() : FormControl(kInvalidFormResourceId) {
}

FormControl::~FormControl() {
}

void FormControl::DispatchChangeEvent() {
  if (!handling_form_event_ && form_) {
    form_->DidChangeFormControl(this);
  }
  EventInit init_dict;
  init_dict.set_bubbles(true);
  init_dict.set_cancelable(false);
  DispatchEvent(new Event(L"change", init_dict));
}

}  // namespace dom
