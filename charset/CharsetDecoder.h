//////////////////////////////////////////////////////////////////////////////
//
// Charset Decoder
//
// Copyright (C) 1996-2012 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evcl3/mainline/listener/winapp/ed_Buffer.h#9 $
//
#if !defined(INCLUDE_charset_CharsetDecoder_h)
#define INCLUDE_charset_CharsetDecoder_h

#include "./Charset.h"

namespace Charset {

class CharsetDecoder {
  public: interface Callback {
    protected: Callback() {}
    public: virtual void DecoderOutput(const char16*, const char16*) = 0;
    DISALLOW_COPY_AND_ASSIGN(Callback);
  };

  // ctor
  protected: CharsetDecoder() {}

  // [C]
  public: static CharsetDecoder* Create(CodePage, Callback*);

  // [F]
  public: virtual void Feed(const char*, const char*) = 0;
  public: virtual void Finish() {}

  DISALLOW_COPY_AND_ASSIGN(CharsetDecoder);
};

} // Charset

#endif // !defined(INCLUDE_charset_CharsetDecoder_h)
