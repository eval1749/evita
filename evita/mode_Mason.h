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
#if !defined(INCLUDE_mode_Mason_h)
#define INCLUDE_mode_Mason_h

#include "./mode_Perl.h"

namespace text
{

/// <summary>
///   Mason mode factory.
/// </summary>
class MasonModeFactory : public ModeFactory
{
    // ctor
    public: MasonModeFactory();

    // [C]
    public: virtual Mode* Create(Buffer*) override;

    // [G]
    protected: virtual const char16* getExtensions() const override
        { return L"m mi"; }

    public: virtual const char16* GetName() const override
        { return L"Mason"; }

    // [I]
    public: virtual bool IsSupported(const char16*) const override;
}; // MasonModeFactory

}  // namespace text

#endif //!defined(INCLUDE_mode_Mason_h)
