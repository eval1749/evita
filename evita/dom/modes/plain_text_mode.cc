// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/modes/plain_text_mode.h"

#include "evita/dom/document.h"
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

  private: static PlainTextMode* NewPlainTextMode(Document* document) {
    return new PlainTextMode(document, text::PlainTextModeFactory::instance());
  }

  DISALLOW_COPY_AND_ASSIGN(PlainTextModeClass);
};
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// Mode
//
DEFINE_SCRIPTABLE_OBJECT(PlainTextMode, PlainTextModeClass);

PlainTextMode::PlainTextMode(Document* document, text::ModeFactory* mode_factory)
    : ScriptableBase(document, mode_factory) {
}

PlainTextMode::~PlainTextMode() {
}

}  // namespace dom
