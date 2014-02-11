// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/modes/config_mode.h"

#include "evita/dom/document.h"
#include "evita/text/modes/config_mode.h"
#include "evita/v8_glue/converter.h"

namespace dom {

namespace {
//////////////////////////////////////////////////////////////////////
//
// ConfigModeClass
//
class ConfigModeClass : public v8_glue::DerivedWrapperInfo<ConfigMode, Mode> {
  public: ConfigModeClass(const char* name)
      : BaseClass(name) {
  }
  public: ~ConfigModeClass() = default;

  private: virtual v8::Handle<v8::FunctionTemplate>
      CreateConstructorTemplate(v8::Isolate* isolate) override {
    return v8_glue::CreateConstructorTemplate(isolate,
        &ConfigModeClass::NewConfigMode);
  }

  private: static ConfigMode* NewConfigMode() {
    return new ConfigMode();
  }

  DISALLOW_COPY_AND_ASSIGN(ConfigModeClass);
};
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// Mode
//
DEFINE_SCRIPTABLE_OBJECT(ConfigMode, ConfigModeClass);

ConfigMode::ConfigMode()
    : ScriptableBase(new text::ConfigMode()) {
}

ConfigMode::~ConfigMode() {
}

}  // namespace dom
