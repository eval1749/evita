// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/forms/checkbox_control.h"

#include "evita/dom/events/form_event.h"
#include "evita/dom/forms/form_control.h"
#include "evita/dom/script_host.h"
#include "evita/dom/view_delegate.h"
#include "evita/v8_glue/converter.h"
#include "evita/v8_glue/wrapper_info.h"

namespace dom {

namespace {
//////////////////////////////////////////////////////////////////////
//
// CheckboxControlClass
//
class CheckboxControlClass :
    public v8_glue::DerivedWrapperInfo<CheckboxControl, FormControl> {

  public: CheckboxControlClass(const char* name)
      : BaseClass(name) {
  }
  public: ~CheckboxControlClass() = default;

  private: virtual v8::Handle<v8::FunctionTemplate>
      CreateConstructorTemplate(v8::Isolate* isolate) override {
    return v8_glue::CreateConstructorTemplate(isolate,
        &CheckboxControlClass::NewCheckboxControl);
  }

  private: static CheckboxControl* NewCheckboxControl(
      FormResourceId control_id) {
    return new CheckboxControl(control_id);
  }

  private: virtual void SetupInstanceTemplate(
      ObjectTemplateBuilder& builder) override {
    builder
        .SetProperty("checked", &CheckboxControl::checked,
            &CheckboxControl::set_checked);
  }

  DISALLOW_COPY_AND_ASSIGN(CheckboxControlClass);
};
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// CheckboxControl
//
DEFINE_SCRIPTABLE_OBJECT(CheckboxControl, CheckboxControlClass);

CheckboxControl::CheckboxControl(FormResourceId control_id)
    : ScriptableBase(control_id), checked_(false) {
}

CheckboxControl::~CheckboxControl() {
}

void CheckboxControl::set_checked(bool new_checked) {
  if (checked_ == new_checked)
    return;
  // TODO(yosi) Dispatch |beforeinput| and |input| event.
  checked_ = new_checked;
  DispatchChangeEvent();
}

bool CheckboxControl::DispatchEvent(Event* event) {
  CR_DEFINE_STATIC_LOCAL(base::string16, kChangeEvent, (L"change"));

  if (auto const form_event = event->as<FormEvent>()) {
    HandlingFormEventScope scope(this);
    if (event->type() == kChangeEvent) {
      auto const new_checked = !form_event->data().empty();
      set_checked(new_checked);
    }
    return false;
  }

  return FormControl::DispatchEvent(event);
}

}  // namespace dom
