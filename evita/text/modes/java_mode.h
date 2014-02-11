// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_text_modes_java_mode_h)
#define INCLUDE_evita_text_modes_java_mode_h

#include "common/memory/singleton.h"
#include "evita/text/modes/mode.h"
#include "evita/text/modes/mode_factory.h"

namespace text {

class JavaModeFactory : public common::Singleton<JavaModeFactory>,
                        public ModeFactory {
  DECLARE_SINGLETON_CLASS(JavaModeFactory);

  private: JavaModeFactory();
  public: ~JavaModeFactory();

  public: virtual Mode* Create() override;
  protected: virtual const char16* getExtensions() const override {
    return L"java";
  }
  public: virtual const char16* GetName() const override { return L"Java"; }

  DISALLOW_COPY_AND_ASSIGN(JavaModeFactory);
};

class JavaMode : public ModeWithLexer {
  public: JavaMode();
  public: virtual ~JavaMode();

  // Mode
  private: virtual const char16* GetName() const override;

  // ModeWithLexer
  private: virtual Lexer* CreateLexer(Buffer* buffer) override;

  DISALLOW_COPY_AND_ASSIGN(JavaMode);
};

}  // namespace text

#endif //!defined(INCLUDE_evita_text_modes_java_mode_h)
