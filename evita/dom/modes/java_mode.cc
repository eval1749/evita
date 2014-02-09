// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/modes/java_mode.h"

#include "evita/dom/document.h"
#include "evita/text/modes/java_mode.h"
#include "evita/v8_glue/converter.h"

namespace dom {

namespace {
//////////////////////////////////////////////////////////////////////
//
// JavaModeClass
//
class JavaModeClass : public v8_glue::DerivedWrapperInfo<JavaMode, Mode> {
  public: JavaModeClass(const char* name)
      : BaseClass(name) {
  }
  public: ~JavaModeClass() = default;

  private: virtual v8::Handle<v8::FunctionTemplate>
      CreateConstructorTemplate(v8::Isolate* isolate) override {
    return v8_glue::CreateConstructorTemplate(isolate,
        &JavaModeClass::NewJavaMode);
  }

  private: static JavaMode* NewJavaMode(Document* document) {
    return new JavaMode(document, text::JavaModeFactory::instance());
  }

  DISALLOW_COPY_AND_ASSIGN(JavaModeClass);
};
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// Mode
//
DEFINE_SCRIPTABLE_OBJECT(JavaMode, JavaModeClass);

JavaMode::JavaMode(Document* document, text::ModeFactory* mode_factory)
    : ScriptableBase(document, mode_factory) {
}

JavaMode::~JavaMode() {
}

}  // namespace dom
