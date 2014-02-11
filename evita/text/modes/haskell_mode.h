//////////////////////////////////////////////////////////////////////////////
//
// Editor - Edit Mode - Haskell Mode
// listener/winapp/mode_Haskell.h
//
// Copyright (C) 1996-2009 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evcl3/mainline/listener/winapp/mode_Haskell.h#1 $
//
#if !defined(INCLUDE_evita_text_modes_haskell_mode_h)
#define INCLUDE_evita_text_modes_haskell_mode_h

#include "evita/text/modes/mode.h"

namespace text {

class HaskellMode : public ModeWithLexer {
  public: HaskellMode();
  public: virtual ~HaskellMode();

  // Mode
  private: virtual const char16* GetName() const override;

  // ModeWithLexer
  private: virtual Lexer* CreateLexer(Buffer* buffer) override;

  DISALLOW_COPY_AND_ASSIGN(HaskellMode);
};

}  // namespace text

#endif //!defined(INCLUDE_evita_text_modes_haskell_mode_h)
