// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/modes/haskell_mode.h"

#include "evita/dom/document.h"
#include "evita/text/modes/haskell_mode.h"
#include "evita/v8_glue/converter.h"

namespace dom {

namespace {
//////////////////////////////////////////////////////////////////////
//
// HaskellModeClass
//
class HaskellModeClass : public v8_glue::DerivedWrapperInfo<HaskellMode, Mode> {
  public: HaskellModeClass(const char* name)
      : BaseClass(name) {
  }
  public: ~HaskellModeClass() = default;

  private: virtual v8::Handle<v8::FunctionTemplate>
      CreateConstructorTemplate(v8::Isolate* isolate) override {
    return v8_glue::CreateConstructorTemplate(isolate,
        &HaskellModeClass::NewHaskellMode);
  }

  private: static HaskellMode* NewHaskellMode() {
    return new HaskellMode();
  }

  DISALLOW_COPY_AND_ASSIGN(HaskellModeClass);
};
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// Mode
//
DEFINE_SCRIPTABLE_OBJECT(HaskellMode, HaskellModeClass);

HaskellMode::HaskellMode()
    : ScriptableBase(text::HaskellModeFactory::instance()) {
}

HaskellMode::~HaskellMode() {
}

}  // namespace dom
