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

#include "evita/text/modes/mode.h"

namespace text {

class PlainTextMode : public Mode {
  public: PlainTextMode();
  public: virtual ~PlainTextMode();

  // Mode
  private: virtual bool DoColor(Count hint) override;
  private: virtual const char16* GetName() const override;

  DISALLOW_COPY_AND_ASSIGN(PlainTextMode);
};

}  // namespace text

#endif //!defined(INCLUDE_evita_text_modes_plain_text_mode_h)
