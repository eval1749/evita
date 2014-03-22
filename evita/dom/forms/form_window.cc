// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/forms/form_window.h"

#include "evita/dom/forms/form.h"
#include "evita/dom/script_controller.h"
#include "evita/dom/view_delegate.h"
#include "evita/v8_glue/constructor_template.h"
#include "evita/v8_glue/converter.h"
#include "evita/v8_glue/function_template_builder.h"
#include "evita/v8_glue/wrapper_info.h"

namespace dom {

namespace {

//////////////////////////////////////////////////////////////////////
//
// FormWindowClass
//
class FormWindowClass :
    public v8_glue::DerivedWrapperInfo<FormWindow, Window> {

  public: explicit FormWindowClass(const char* name)
      : BaseClass(name) {
  }
  public: ~FormWindowClass() = default;

  private: virtual v8::Handle<v8::FunctionTemplate>
      CreateConstructorTemplate(v8::Isolate* isolate) override {
    auto templ = v8_glue::CreateConstructorTemplate(isolate,
        &FormWindowClass::NewFormWindow);
    return v8_glue::FunctionTemplateBuilder(isolate, templ)
        .Build();
  }

  private: static FormWindow* NewFormWindow(Form* form) {
    return new FormWindow(form);
  }

  private: virtual void SetupInstanceTemplate(
      ObjectTemplateBuilder& builder) override {
    builder.SetProperty("form", &FormWindow::form);
  }

  DISALLOW_COPY_AND_ASSIGN(FormWindowClass);
};
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// FormWindow
//
DEFINE_SCRIPTABLE_OBJECT(FormWindow, FormWindowClass);

FormWindow::FormWindow(Form* form) : form_(form) {
  ScriptController::instance()->view_delegate()->CreateFormWindow(
      window_id(), form);
}

FormWindow::~FormWindow() {
}

}  // namespace dom
