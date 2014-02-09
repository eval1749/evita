// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/modes/cxx_mode.h"

#include "evita/dom/document.h"
#include "evita/text/modes/cxx_mode.h"
#include "evita/v8_glue/converter.h"

namespace dom {

namespace {
//////////////////////////////////////////////////////////////////////
//
// CxxModeClass
//
class CxxModeClass : public v8_glue::DerivedWrapperInfo<CxxMode, Mode> {
  public: CxxModeClass(const char* name)
      : BaseClass(name) {
  }
  public: ~CxxModeClass() = default;

  private: virtual v8::Handle<v8::FunctionTemplate>
      CreateConstructorTemplate(v8::Isolate* isolate) override {
    return v8_glue::CreateConstructorTemplate(isolate,
        &CxxModeClass::NewCxxMode);
  }

  private: static CxxMode* NewCxxMode() {
    return new CxxMode();
  }

  DISALLOW_COPY_AND_ASSIGN(CxxModeClass);
};
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// Mode
//
DEFINE_SCRIPTABLE_OBJECT(CxxMode, CxxModeClass);

CxxMode::CxxMode()
    : ScriptableBase(text::CxxModeFactory::instance()) {
}

CxxMode::~CxxMode() {
}

}  // namespace dom
