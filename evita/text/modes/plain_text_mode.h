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
#if !defined(INCLUDE_evita_text_modes_plain_text_mode_h)
#define INCLUDE_evita_text_modes_plain_text_mode_h

#include "common/memory/singleton.h"
#include "evita/text/modes/mode.h"
#include "evita/text/modes/mode_factory.h"

namespace text
{

//////////////////////////////////////////////////////////////////////
//
// PlainTextMode
//
class PlainTextMode : public Mode {
  public: PlainTextMode();
  public: ~PlainTextMode();

  public: virtual bool DoColor(Count) override;
  public: virtual const char16* GetName() const override;

  DISALLOW_COPY_AND_ASSIGN(PlainTextMode);
}; // PlainTextMode

//////////////////////////////////////////////////////////////////////
//
// PlainTextModeFactory
//
class PlainTextModeFactory : public common::Singleton<PlainTextModeFactory>,
                       public ModeFactory {
    DECLARE_SINGLETON_CLASS(PlainTextModeFactory);

    // ctor
    private: PlainTextModeFactory();
    public: ~PlainTextModeFactory();

    // [C]
    public: virtual Mode* Create() override;

    // [G]
    protected: virtual const char16* getExtensions() const override
        { return L"txt"; }

    public: virtual const char16* GetName() const override
        { return L"Plain"; }

    DISALLOW_COPY_AND_ASSIGN(PlainTextModeFactory);
}; // PlainTextModeFactory

}  // namespace text

#endif //!defined(INCLUDE_evita_text_modes_plain_text_mode_h)
