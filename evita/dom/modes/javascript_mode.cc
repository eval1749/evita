// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/modes/javascript_mode.h"

#include "evita/dom/document.h"
#include "evita/text/modes/javascript_mode.h"
#include "evita/v8_glue/converter.h"

namespace dom {

namespace {
//////////////////////////////////////////////////////////////////////
//
// JavaScriptModeClass
//
class JavaScriptModeClass
    : public v8_glue::DerivedWrapperInfo<JavaScriptMode, Mode> {
  public: JavaScriptModeClass(const char* name)
      : BaseClass(name) {
  }
  public: ~JavaScriptModeClass() = default;

  private: virtual v8::Handle<v8::FunctionTemplate>
      CreateConstructorTemplate(v8::Isolate* isolate) override {
    return v8_glue::CreateConstructorTemplate(isolate,
        &JavaScriptModeClass::NewJavaScriptMode);
  }

  private: static JavaScriptMode* NewJavaScriptMode(Document* document) {
    return new JavaScriptMode(document, text::JavaScriptModeFactory::instance());
  }

  DISALLOW_COPY_AND_ASSIGN(JavaScriptModeClass);
};
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// Mode
//
DEFINE_SCRIPTABLE_OBJECT(JavaScriptMode, JavaScriptModeClass);

JavaScriptMode::JavaScriptMode(Document* document, text::ModeFactory* mode_factory)
    : ScriptableBase(document, mode_factory) {
}

JavaScriptMode::~JavaScriptMode() {
}

}  // namespace dom
