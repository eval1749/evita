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
#if !defined(INCLUDE_mode_Perl_h)
#define INCLUDE_mode_Perl_h

#include "./ed_Mode.h"

#include "evita/text/buffer.h"

namespace text
{

/// <summary>
///   PerlMode factory
/// </summary>
class PerlModeFactory : public ModeFactory
{
    // ctor
    public: PerlModeFactory();

    // [C]
    public: virtual Mode* Create(Buffer*) override;

    // [G]
    protected: virtual const char16* getExtensions() const override
        { return L"pl pm t"; }

    public: virtual const char16* GetName() const override
        { return L"Perl"; }
}; // PerlModeFactory

}  // namespace text

#endif //!defined(INCLUDE_mode_Perl_h)
