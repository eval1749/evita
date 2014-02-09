// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/modes/python_mode.h"

#include "evita/dom/document.h"
#include "evita/text/modes/python_mode.h"
#include "evita/v8_glue/converter.h"

namespace dom {

namespace {
//////////////////////////////////////////////////////////////////////
//
// PythonModeClass
//
class PythonModeClass : public v8_glue::DerivedWrapperInfo<PythonMode, Mode> {
  public: PythonModeClass(const char* name)
      : BaseClass(name) {
  }
  public: ~PythonModeClass() = default;

  private: virtual v8::Handle<v8::FunctionTemplate>
      CreateConstructorTemplate(v8::Isolate* isolate) override {
    return v8_glue::CreateConstructorTemplate(isolate,
        &PythonModeClass::NewPythonMode);
  }

  private: static PythonMode* NewPythonMode() {
    return new PythonMode();
  }

  DISALLOW_COPY_AND_ASSIGN(PythonModeClass);
};
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// Mode
//
DEFINE_SCRIPTABLE_OBJECT(PythonMode, PythonModeClass);

PythonMode::PythonMode()
    : ScriptableBase(text::PythonModeFactory::instance()) {
}

PythonMode::~PythonMode() {
}

}  // namespace dom
