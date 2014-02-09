// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_text_modes_javascript_mode_h)
#define INCLUDE_evita_text_modes_javascript_mode_h

#include "common/memory/singleton.h"
#include "evita/text/modes/mode_factory.h"
#include "evita/text/buffer.h"

namespace text {

class JavaScriptModeFactory : public common::Singleton<JavaScriptModeFactory>,
                        public ModeFactory {
  DECLARE_SINGLETON_CLASS(JavaScriptModeFactory);

  private: JavaScriptModeFactory();
  public: ~JavaScriptModeFactory();

  public: virtual Mode* Create(Buffer*) override;
  protected: virtual const char16* getExtensions() const override {
    return L"js jsm";
  }
  public: virtual const char16* GetName() const override {
    return L"JavaScript";
  }

  DISALLOW_COPY_AND_ASSIGN(JavaScriptModeFactory);
};

}  // namespace text

#endif //!defined(INCLUDE_evita_text_modes_javascript_mode_h)
