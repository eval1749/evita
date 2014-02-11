//////////////////////////////////////////////////////////////////////////////
//
// Editor - Edit Mode - XML Mode
// listener/winapp/mode_Xml.h
//
// Copyright (C) 1996-2007 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evcl3/mainline/listener/winapp/mode_Xml.h#3 $
//
#if !defined(INCLUDE_evita_text_modes_xml_mode_h)
#define INCLUDE_evita_text_modes_xml_mode_h

#include "evita/text/modes/mode.h"

namespace text {

//////////////////////////////////////////////////////////////////////
//
// XmlMode
//
class XmlMode : public ModeWithLexer {
  public: XmlMode();
  public: virtual ~XmlMode();

  // Mode
  private: virtual const char16* GetName() const override;

  // ModeWithLexer
  private: virtual Lexer* CreateLexer(Buffer* buffer) override;

  DISALLOW_COPY_AND_ASSIGN(XmlMode);
};

}  // namespace text

#endif //!defined(INCLUDE_evita_text_modes_xml_mode_h)
