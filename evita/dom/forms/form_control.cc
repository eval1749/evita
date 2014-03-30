// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/forms/form_control.h"

#include "evita/dom/events/event.h"
#include "evita/dom/events/event_init.h"
#include "evita/dom/forms/form.h"
#include "evita/dom/script_host.h"
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
    public v8_glue::DerivedWrapperInfo<FormControl, ViewEventTarget> {

  public: FormControlClass(const char* name)
      : BaseClass(name) {
  }
  public: ~FormControlClass() = default;

  private: virtual void SetupInstanceTemplate(
      ObjectTemplateBuilder& builder) override {
    builder
        .SetProperty("clientHeight", &FormControl::client_height,
                                     &FormControl::set_client_height)
        .SetProperty("clientLeft", &FormControl::client_left,
                                     &FormControl::set_client_left)
        .SetProperty("clientTop", &FormControl::client_top,
                                     &FormControl::set_client_top)
        .SetProperty("clientWidth", &FormControl::client_width,
                                     &FormControl::set_client_width)
        .SetProperty("controlId", &FormControl::control_id)
        .SetProperty("disabled", &FormControl::disabled,
                                 &FormControl::set_disabled)
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
    : control_id_(control_id), disabled_(false), handling_form_event_(false),
      name_(name) {
}

FormControl::FormControl(FormResourceId control_id)
    : FormControl(control_id, base::string16()) {
}

FormControl::FormControl() : FormControl(kInvalidFormResourceId) {
  DCHECK_EQ(kInvalidFormResourceId, control_id_);
  control_id_ = event_target_id();
}

FormControl::~FormControl() {
}

void FormControl::set_client_height(float new_height) {
  if (rect_.height() == new_height)
    return;
  rect_.bottom = rect_.top + new_height;
  NotifyControlChange();
}

void FormControl::set_client_left(float new_left) {
  if (rect_.left == new_left)
    return;
  rect_.left = new_left;
  NotifyControlChange();
}

void FormControl::set_client_top(float new_top) {
  if (rect_.top == new_top)
    return;
  rect_.top = new_top;
  NotifyControlChange();
}

void FormControl::set_client_width(float new_width) {
  if (rect_.width() == new_width)
    return;
  rect_.right = rect_.left + new_width;
  NotifyControlChange();
}

void FormControl::set_disabled(bool new_disabled) {
  if (disabled_ == new_disabled)
    return;
  disabled_ = new_disabled;
  NotifyControlChange();
}

void FormControl::DispatchChangeEvent() {
  if (!handling_form_event_ && form_)
    NotifyControlChange();
  EventInit init_dict;
  init_dict.set_bubbles(true);
  init_dict.set_cancelable(false);
  DispatchEvent(new Event(L"change", init_dict));
}

void FormControl::NotifyControlChange() {
  if (!form_)
    return;
  form_->DidChangeFormControl(this);
}

// dom::EventTarget
EventTarget::EventPath FormControl::BuildEventPath() const {
  return form_ ? EventPath {form_} : EventPath();
}

}  // namespace dom
