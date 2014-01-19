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
#if !defined(INCLUDE_mode_Lisp_h)
#define INCLUDE_mode_Lisp_h

#include "evita/ed_Mode.h"

namespace text
{

/// <remark>
///   Lisp mode facotry
/// </remark>
class LispModeFactory : public ModeFactory
{
    // ctor
    public: LispModeFactory();

    // [C]
    public: virtual Mode* Create(Buffer*) override;

    // [G]
    protected: virtual const char16* getExtensions() const override
        { return L"cl l lisp lsp scm el"; }

    public: virtual const char16* GetName() const override
        { return L"Lisp"; }
}; // LispModeFactory

}  // namespace text

#endif //!defined(INCLUDE_mode_Lisp_h)
