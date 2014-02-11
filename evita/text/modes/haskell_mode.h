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

#include "common/memory/singleton.h"
#include "evita/text/modes/mode.h"
#include "evita/text/modes/mode_factory.h"

namespace text
{

/// <summary>
///  Haskell mode factory
/// </summary>
class HaskellModeFactory : public common::Singleton<HaskellModeFactory>,
                       public ModeFactory {
    DECLARE_SINGLETON_CLASS(HaskellModeFactory);

    // ctor
    private: HaskellModeFactory();
    public: ~HaskellModeFactory();

    // [C]
    public: virtual Mode* Create() override;

    // [G]
    protected: virtual const char16* getExtensions() const override
        { return L"hs hsc"; }

    public: virtual const char16* GetName() const override
        { return L"Haskell"; }

    DISALLOW_COPY_AND_ASSIGN(HaskellModeFactory);
}; // HaskellModeFactory

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
