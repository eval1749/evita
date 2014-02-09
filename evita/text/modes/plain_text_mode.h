//////////////////////////////////////////////////////////////////////////////
//
// Editor - Edit Mode - Plain Text
// listener/winapp/mode_PlainText.h
//
// Copyright (C) 1996-2007 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evcl3/mainline/listener/winapp/mode_PlainText.h#2 $
//
#if !defined(INCLUDE_mode_PlainText_h)
#define INCLUDE_mode_PlainText_h

#include "evita/text/modes/mode.h"
#include "evita/text/modes/mode_factory.h"

namespace text
{

//////////////////////////////////////////////////////////////////////
//
// PlainTextMode
//
class PlainTextMode : public Mode
{
    // ctor
    public: PlainTextMode(ModeFactory*, Buffer*);

    // [D]
    public: virtual bool DoColor(Count) override { return false; }

    DISALLOW_COPY_AND_ASSIGN(PlainTextMode);
}; // PlainTextMode


//////////////////////////////////////////////////////////////////////
//
// PlainTextModeFactory
//
class PlainTextModeFactory : public ModeFactory
{
    // ctor
    public: PlainTextModeFactory();

    // [C]
    public: virtual Mode* Create(Buffer* pBuffer) override
        { return new PlainTextMode(this, pBuffer); }

    // [G]
    protected: virtual const char16* getExtensions() const override
        { return L"txt"; }

    public: virtual const char16* GetName() const override
        { return L"Plain"; }

    DISALLOW_COPY_AND_ASSIGN(PlainTextModeFactory);
}; // PlainTextModeFactory

}  // namespace text

#endif //!defined(INCLUDE_mode_PlainText_h)
