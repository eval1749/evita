// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/forms/radio_button_control.h"

#include "evita/dom/events/form_event.h"
#include "evita/dom/forms/form_control.h"
#include "evita/dom/script_controller.h"
#include "evita/dom/view_delegate.h"
#include "evita/v8_glue/converter.h"
#include "evita/v8_glue/wrapper_info.h"

namespace dom {

namespace {
//////////////////////////////////////////////////////////////////////
//
// RadioButtonControlClass
//
class RadioButtonControlClass :
    public v8_glue::DerivedWrapperInfo<RadioButtonControl, FormControl> {

  public: RadioButtonControlClass(const char* name)
      : BaseClass(name) {
  }
  public: ~RadioButtonControlClass() = default;

  private: virtual v8::Handle<v8::FunctionTemplate>
      CreateConstructorTemplate(v8::Isolate* isolate) override {
    return v8_glue::CreateConstructorTemplate(isolate,
        &RadioButtonControlClass::NewRadioButtonControl);
  }

  private: static RadioButtonControl* NewRadioButtonControl(
      FormResourceId control_id) {
    return new RadioButtonControl(control_id);
  }

  private: virtual void SetupInstanceTemplate(
      ObjectTemplateBuilder& builder) override {
    builder
        .SetProperty("checked", &RadioButtonControl::checked,
            &RadioButtonControl::set_checked);
  }

  DISALLOW_COPY_AND_ASSIGN(RadioButtonControlClass);
};
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// RadioButtonControl
//
DEFINE_SCRIPTABLE_OBJECT(RadioButtonControl, RadioButtonControlClass);

RadioButtonControl::RadioButtonControl(FormResourceId control_id)
    : ScriptableBase(control_id), checked_(false) {
}

RadioButtonControl::~RadioButtonControl() {
}

bool RadioButtonControl::DispatchEvent(Event* event) {
  CR_DEFINE_STATIC_LOCAL(base::string16, kChangeEvent, (L"change"));

  if (auto const form_event = event->as<FormEvent>()) {
    if (event->type() == kChangeEvent)
      checked_ = !form_event->data().empty();
  }

  return FormControl::DispatchEvent(event);
}

}  // namespace dom
