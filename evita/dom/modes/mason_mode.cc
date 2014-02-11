// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/modes/mason_mode.h"

#include "evita/dom/document.h"
#include "evita/text/modes/mason_mode.h"
#include "evita/v8_glue/converter.h"

namespace dom {

namespace {
//////////////////////////////////////////////////////////////////////
//
// MasonModeClass
//
class MasonModeClass : public v8_glue::DerivedWrapperInfo<MasonMode, Mode> {
  public: MasonModeClass(const char* name)
      : BaseClass(name) {
  }
  public: ~MasonModeClass() = default;

  private: virtual v8::Handle<v8::FunctionTemplate>
      CreateConstructorTemplate(v8::Isolate* isolate) override {
    return v8_glue::CreateConstructorTemplate(isolate,
        &MasonModeClass::NewMasonMode);
  }

  private: static MasonMode* NewMasonMode() {
    return new MasonMode();
  }

  DISALLOW_COPY_AND_ASSIGN(MasonModeClass);
};
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// Mode
//
DEFINE_SCRIPTABLE_OBJECT(MasonMode, MasonModeClass);

MasonMode::MasonMode()
    : ScriptableBase(new text::MasonMode()) {
}

MasonMode::~MasonMode() {
}

}  // namespace dom
