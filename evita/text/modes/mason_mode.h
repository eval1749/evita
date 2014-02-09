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

#include "common/memory/singleton.h"
#include "evita/text/modes/perl_mode.h"

namespace text
{

class MasonModeFactory : public common::Singleton<MasonModeFactory>,
                       public ModeFactory {
    DECLARE_SINGLETON_CLASS(MasonModeFactory);

    // ctor
    private: MasonModeFactory();
    public: ~MasonModeFactory();

    // [C]
    public: virtual Mode* Create() override;

    // [G]
    protected: virtual const char16* getExtensions() const override
        { return L"m mi"; }

    public: virtual const char16* GetName() const override
        { return L"Mason"; }

    // [I]
    public: virtual bool IsSupported(const char16*) const override;
}; // MasonModeFactory

}  // namespace text

#endif //!defined(INCLUDE_evita_text_modes_mason_mode_h)
