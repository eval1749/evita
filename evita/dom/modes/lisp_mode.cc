// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/modes/lisp_mode.h"

#include "evita/dom/document.h"
#include "evita/text/modes/lisp_mode.h"
#include "evita/v8_glue/converter.h"

namespace dom {

namespace {
//////////////////////////////////////////////////////////////////////
//
// LispModeClass
//
class LispModeClass : public v8_glue::DerivedWrapperInfo<LispMode, Mode> {
  public: LispModeClass(const char* name)
      : BaseClass(name) {
  }
  public: ~LispModeClass() = default;

  private: virtual v8::Handle<v8::FunctionTemplate>
      CreateConstructorTemplate(v8::Isolate* isolate) override {
    return v8_glue::CreateConstructorTemplate(isolate,
        &LispModeClass::NewLispMode);
  }

  private: static LispMode* NewLispMode(Document* document) {
    return new LispMode(document, text::LispModeFactory::instance());
  }

  DISALLOW_COPY_AND_ASSIGN(LispModeClass);
};
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// Mode
//
DEFINE_SCRIPTABLE_OBJECT(LispMode, LispModeClass);

LispMode::LispMode(Document* document, text::ModeFactory* mode_factory)
    : ScriptableBase(document, mode_factory) {
}

LispMode::~LispMode() {
}

}  // namespace dom
