// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/forms/form_control.h"

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
        .SetProperty("resouce_id", &FormControl::resouce_id);
  }
};
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// FormControl
//
DEFINE_SCRIPTABLE_OBJECT(FormControl, FormControlClass);

FormControl::FormControl(FormResourceId resouce_id)
    : resouce_id_(resouce_id) {
}

FormControl::FormControl() : FormControl(kInvalidFormResourceId) {
}

FormControl::~FormControl() {
}

}  // namespace dom
