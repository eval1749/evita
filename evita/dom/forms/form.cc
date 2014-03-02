// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/forms/form.h"

#include "evita/dom/forms/form_control.h"
#include "evita/dom/script_controller.h"
#include "evita/dom/view_delegate.h"
#include "evita/v8_glue/converter.h"
#include "evita/v8_glue/wrapper_info.h"

namespace dom {

namespace {
//////////////////////////////////////////////////////////////////////
//
// FormClass
//
class FormClass :
    public v8_glue::DerivedWrapperInfo<Form, EventTarget> {

  public: FormClass(const char* name)
      : BaseClass(name) {
  }
  public: ~FormClass() = default;

  private: virtual v8::Handle<v8::FunctionTemplate>
      CreateConstructorTemplate(v8::Isolate* isolate) override {
    return v8_glue::CreateConstructorTemplate(isolate,
        &FormClass::NewForm);
  }

  private: static Form* NewForm() {
    return new Form();
  }

  private: virtual void SetupInstanceTemplate(
      ObjectTemplateBuilder& builder) override {
    builder
        .SetProperty("controls", &Form::controls)
        .SetMethod("add", &Form::AddFormControl)
        .SetMethod("realize", &Form::Realize)
        .SetMethod("show", &Form::Show);
  }

  DISALLOW_COPY_AND_ASSIGN(FormClass);
};
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// Form
//
DEFINE_SCRIPTABLE_OBJECT(Form, FormClass);

Form::Form() {
  ScriptController::instance()->view_delegate()->CreateDialogBox(this);
}

Form::~Form() {
}

void Form::AddFormControl(FormControl* control) {
  controls_.push_back(control);
  control->form_ = this;
}

void Form::Realize() {
  ScriptController::instance()->view_delegate()->RealizeDialogBox(
      dialog_box_id());
}

void Form::Show() {
  ScriptController::instance()->view_delegate()->ShowDialogBox(
      dialog_box_id());
}

}  // namespace dom
