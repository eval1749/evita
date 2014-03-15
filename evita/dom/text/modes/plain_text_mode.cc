// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/text/modes/plain_text_mode.h"

#include "evita/dom/text/document.h"
#include "evita/text/modes/plain_text_mode.h"
#include "evita/v8_glue/converter.h"

namespace dom {

namespace {
//////////////////////////////////////////////////////////////////////
//
// PlainTextModeClass
//
class PlainTextModeClass
    : public v8_glue::DerivedWrapperInfo<PlainTextMode, Mode> {
  public: PlainTextModeClass(const char* name)
      : BaseClass(name) {
  }
  public: ~PlainTextModeClass() = default;

  private: virtual v8::Handle<v8::FunctionTemplate>
      CreateConstructorTemplate(v8::Isolate* isolate) override {
    return v8_glue::CreateConstructorTemplate(isolate,
        &PlainTextModeClass::NewPlainTextMode);
  }

  private: static PlainTextMode* NewPlainTextMode() {
    return new PlainTextMode();
  }

  DISALLOW_COPY_AND_ASSIGN(PlainTextModeClass);
};
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// Mode
//
DEFINE_SCRIPTABLE_OBJECT(PlainTextMode, PlainTextModeClass);

PlainTextMode::PlainTextMode()
    : ScriptableBase(new text::PlainTextMode()) {
}

PlainTextMode::~PlainTextMode() {
}

}  // namespace dom
