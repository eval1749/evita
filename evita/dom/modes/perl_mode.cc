// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/modes/perl_mode.h"

#include "evita/dom/document.h"
#include "evita/text/modes/perl_mode.h"
#include "evita/v8_glue/converter.h"

namespace dom {

namespace {
//////////////////////////////////////////////////////////////////////
//
// PerlModeClass
//
class PerlModeClass : public v8_glue::DerivedWrapperInfo<PerlMode, Mode> {
  public: PerlModeClass(const char* name)
      : BaseClass(name) {
  }
  public: ~PerlModeClass() = default;

  private: virtual v8::Handle<v8::FunctionTemplate>
      CreateConstructorTemplate(v8::Isolate* isolate) override {
    return v8_glue::CreateConstructorTemplate(isolate,
        &PerlModeClass::NewPerlMode);
  }

  private: static PerlMode* NewPerlMode() {
    return new PerlMode();
  }

  DISALLOW_COPY_AND_ASSIGN(PerlModeClass);
};
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// Mode
//
DEFINE_SCRIPTABLE_OBJECT(PerlMode, PerlModeClass);

PerlMode::PerlMode()
    : ScriptableBase(new text::PerlMode()) {
}

PerlMode::~PerlMode() {
}

}  // namespace dom
