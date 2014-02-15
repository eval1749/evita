// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/forms/text_field_control.h"

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
// TextFieldControlClass
//
class TextFieldControlClass :
    public v8_glue::DerivedWrapperInfo<TextFieldControl, FormControl> {

  public: TextFieldControlClass(const char* name)
      : BaseClass(name) {
  }
  public: ~TextFieldControlClass() = default;

  private: virtual v8::Handle<v8::FunctionTemplate>
      CreateConstructorTemplate(v8::Isolate* isolate) override {
    return v8_glue::CreateConstructorTemplate(isolate,
        &TextFieldControlClass::NewTextFieldControl);
  }

  private: static TextFieldControl* NewTextFieldControl(
      FormResourceId control_id) {
    return new TextFieldControl(control_id);
  }

  private: virtual void SetupInstanceTemplate(
      ObjectTemplateBuilder& builder) override {
    builder
        .SetProperty("value", &TextFieldControl::value,
            &TextFieldControl::set_value);
  }

  DISALLOW_COPY_AND_ASSIGN(TextFieldControlClass);
};
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// TextFieldControl
//
DEFINE_SCRIPTABLE_OBJECT(TextFieldControl, TextFieldControlClass);

TextFieldControl::TextFieldControl(FormResourceId control_id)
    : ScriptableBase(control_id) {
}

TextFieldControl::~TextFieldControl() {
}

bool TextFieldControl::DispatchEvent(Event* event) {
  CR_DEFINE_STATIC_LOCAL(base::string16, kChangeEvent, (L"change"));

  if (auto const form_event = event->as<FormEvent>()) {
    if (event->type() == kChangeEvent)
      value_ = form_event->data();
  }

  return FormControl::DispatchEvent(event);
}

}  // namespace dom
