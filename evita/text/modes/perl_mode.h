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

#include "common/memory/singleton.h"
#include "evita/text/modes/mode.h"
#include "evita/text/modes/mode_factory.h"

namespace text
{

class PerlModeFactory : public common::Singleton<PerlModeFactory>,
                       public ModeFactory {
    DECLARE_SINGLETON_CLASS(PerlModeFactory);

    // ctor
    private: ~PerlModeFactory();
    public: PerlModeFactory();

    // [C]
    public: virtual Mode* Create() override;

    // [G]
    protected: virtual const char16* getExtensions() const override
        { return L"pl pm t"; }

    public: virtual const char16* GetName() const override
        { return L"Perl"; }

    DISALLOW_COPY_AND_ASSIGN(PerlModeFactory);
}; // PerlModeFactory

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
