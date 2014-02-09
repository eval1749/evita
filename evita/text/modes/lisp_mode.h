//////////////////////////////////////////////////////////////////////////////
//
// Editor - Edit Mode - Plain Text
// listener/winapp/mode_Lisp.h
//
// Copyright (C) 1996-2007 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evcl3/mainline/listener/winapp/mode_Lisp.h#3 $
//
#if !defined(INCLUDE_evita_text_modes_lisp_mode_h)
#define INCLUDE_evita_text_modes_lisp_mode_h

#include "common/memory/singleton.h"
#include "evita/text/modes/mode_factory.h"

namespace text
{

/// <remark>
///   Lisp mode facotry
/// </remark>
class LispModeFactory : public common::Singleton<LispModeFactory>,
                       public ModeFactory {
    DECLARE_SINGLETON_CLASS(LispModeFactory);

    // ctor
    private: ~LispModeFactory();
    public: LispModeFactory();

    // [C]
    public: virtual Mode* Create() override;

    // [G]
    protected: virtual const char16* getExtensions() const override
        { return L"cl l lisp lsp scm el"; }

    public: virtual const char16* GetName() const override
        { return L"Lisp"; }


    DISALLOW_COPY_AND_ASSIGN(LispModeFactory);
}; // LispModeFactory

}  // namespace text

#endif //!defined(INCLUDE_evita_text_modes_lisp_mode_h)
