//////////////////////////////////////////////////////////////////////////////
//
// Editor - Edit Mode - C++
// listener/winapp/mode_Perl.h
//
// Copyright (C) 1996-2007 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evcl3/mainline/listener/winapp/mode_Perl.h#4 $
//
#if !defined(INCLUDE_evita_text_modes_perl_mode_h)
#define INCLUDE_evita_text_modes_perl_mode_h

#include "evita/text/modes/mode.h"

namespace text {

class PerlMode : public ModeWithLexer {
  public: PerlMode();
  public: virtual ~PerlMode();

  // Mode
  private: virtual const char16* GetName() const override;

  // ModeWithLexer
  private: virtual Lexer* CreateLexer(Buffer* buffer) override;

  DISALLOW_COPY_AND_ASSIGN(PerlMode);
};

}  // namespace text

#endif //!defined(INCLUDE_evita_text_modes_perl_mode_h)
