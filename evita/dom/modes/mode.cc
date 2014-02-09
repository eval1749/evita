// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/modes/mode.h"

#include "evita/dom/buffer.h"
#include "evita/dom/document.h"
#include "evita/text/modes/mode.h"
#include "evita/text/modes/mode_factory.h"
#include "evita/v8_glue/converter.h"

namespace dom {

namespace {
//////////////////////////////////////////////////////////////////////
//
// ModeClass
//
class ModeClass : public v8_glue::WrapperInfo {
  public: ModeClass(const char* name)
      : v8_glue::WrapperInfo(name) {
  }
  public: ~ModeClass() = default;

  private: virtual void SetupInstanceTemplate(
      ObjectTemplateBuilder& builder) override {
    builder
        .SetProperty("name", &Mode::name)
        .SetMethod("doColor_", &Mode::DoColor);
  }

  DISALLOW_COPY_AND_ASSIGN(ModeClass);
};
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// Mode
//
DEFINE_SCRIPTABLE_OBJECT(Mode, ModeClass);

Mode::Mode(text::ModeFactory* mode_factory)
    : mode_(mode_factory->Create()) {
}

Mode::~Mode() {
}

base::string16 Mode::name() const {
  return mode_->GetName();
}

void Mode::DoColor(int hint) {
  mode_->DoColor(hint);
}

}  // namespace dom
