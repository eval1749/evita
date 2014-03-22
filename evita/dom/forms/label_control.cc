// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/forms/label_control.h"

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
// LabelControlClass
//
class LabelControlClass :
    public v8_glue::DerivedWrapperInfo<LabelControl, FormControl> {

  public: LabelControlClass(const char* name)
      : BaseClass(name) {
  }
  public: ~LabelControlClass() = default;

  private: virtual v8::Handle<v8::FunctionTemplate>
      CreateConstructorTemplate(v8::Isolate* isolate) override {
    return v8_glue::CreateConstructorTemplate(isolate,
        &LabelControlClass::NewLabelControl);
  }

  private: static LabelControl* NewLabelControl(const base::string16& text) {
    return new LabelControl(text);
  }

  private: virtual void SetupInstanceTemplate(
      ObjectTemplateBuilder& builder) override {
    builder.SetProperty("text", &LabelControl::text, &LabelControl::set_text);
  }

  DISALLOW_COPY_AND_ASSIGN(LabelControlClass);
};
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// LabelControl
//
DEFINE_SCRIPTABLE_OBJECT(LabelControl, LabelControlClass);

// TODO(yosi) Once we get rid control_id, we should remove this.
static int label_control_id;

LabelControl::LabelControl(const base::string16& text)
    : ScriptableBase(++label_control_id), text_(text) {
}

LabelControl::~LabelControl() {
}

void LabelControl::set_text(const base::string16& new_text) {
  if (text_ == new_text)
    return;
  text_ = new_text;
  NotifyControlChange();
}

}  // namespace dom
