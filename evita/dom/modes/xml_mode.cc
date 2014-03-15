// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/modes/xml_mode.h"

#include "evita/dom/text/document.h"
#include "evita/text/modes/xml_mode.h"
#include "evita/v8_glue/converter.h"

namespace dom {

namespace {
//////////////////////////////////////////////////////////////////////
//
// XmlModeClass
//
class XmlModeClass : public v8_glue::DerivedWrapperInfo<XmlMode, Mode> {
  public: XmlModeClass(const char* name)
      : BaseClass(name) {
  }
  public: ~XmlModeClass() = default;

  private: virtual v8::Handle<v8::FunctionTemplate>
      CreateConstructorTemplate(v8::Isolate* isolate) override {
    return v8_glue::CreateConstructorTemplate(isolate,
        &XmlModeClass::NewXmlMode);
  }

  private: static XmlMode* NewXmlMode() {
    return new XmlMode();
  }

  DISALLOW_COPY_AND_ASSIGN(XmlModeClass);
};
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// Mode
//
DEFINE_SCRIPTABLE_OBJECT(XmlMode, XmlModeClass);

XmlMode::XmlMode()
    : ScriptableBase(new text::XmlMode()) {
}

XmlMode::~XmlMode() {
}

}  // namespace dom
