// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_text_modes_javascript_mode_h)
#define INCLUDE_evita_text_modes_javascript_mode_h

#include "evita/text/modes/mode.h"

namespace text {

class JavaScriptMode : public ModeWithLexer {
  public: JavaScriptMode();
  public: virtual ~JavaScriptMode();

  // Mode
  private: virtual const char16* GetName() const override;

  // ModeWithLexer
  private: virtual Lexer* CreateLexer(Buffer* buffer) override;

  DISALLOW_COPY_AND_ASSIGN(JavaScriptMode);
};

}  // namespace text

#endif //!defined(INCLUDE_evita_text_modes_javascript_mode_h)
