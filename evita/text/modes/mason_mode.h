//////////////////////////////////////////////////////////////////////////////
//
// Editor - Edit Mode - C++ Mode
// listener/winapp/mode_Mason.h
//
// Copyright (C) 1996-2007 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evcl3/mainline/listener/winapp/mode_Mason.h#3 $
//
#if !defined(INCLUDE_evita_text_modes_mason_mode_h)
#define INCLUDE_evita_text_modes_mason_mode_h

#include "evita/text/modes/mode.h"

namespace text {

class MasonMode : public ModeWithLexer {
  public: MasonMode();
  public: virtual ~MasonMode();

  // Mode
  private: virtual const char16* GetName() const override;

  // ModeWithLexer
  private: virtual Lexer* CreateLexer(Buffer* buffer) override;

  DISALLOW_COPY_AND_ASSIGN(MasonMode);
};

}  // namespace text

#endif //!defined(INCLUDE_evita_text_modes_mason_mode_h)
