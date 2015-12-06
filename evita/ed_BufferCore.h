//////////////////////////////////////////////////////////////////////////////
//
// evcl - listener - edit buffer
// listener/winapp/ed_buffer.h
//
// Copyright (C) 1996-2007 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evcl3/mainline/listener/winapp/ed_BufferCore.h#1 $
//
#ifndef EVITA_ED_BUFFERCORE_H_
#define EVITA_ED_BUFFERCORE_H_

#include "base/strings/string16.h"
#include "evita/precomp.h"

namespace text {

//////////////////////////////////////////////////////////////////////
//
// BufferCore
//
class BufferCore {
 public:
  ~BufferCore();

  bool operator==(const BufferCore* other) const { return this == other; }

  bool operator!=(const BufferCore* other) const { return this != other; }

  // [E]
  Posn EnsurePosn(Posn lPosn) const {
    if (lPosn < 0)
      return 0;
    if (lPosn > m_lEnd)
      return m_lEnd;
    return lPosn;
  }

  // [G]
  base::char16 GetCharAt(Posn) const;
  Posn GetEnd() const { return m_lEnd; }
  Count GetText(base::char16*, Posn, Posn) const;
  base::string16 GetText(Posn start, Posn end) const;

  // [I]
  bool IsValidPosn(Posn p) const { return p >= 0 && p <= m_lEnd; }

  bool IsValidRange(Posn s, Posn e) const {
    return IsValidPosn(s) && IsValidPosn(e) && s <= e;
  }

 protected:
  BufferCore();

  Count deleteChars(Posn, Posn);
  void extend(Posn, int);
  void insert(Posn, const base::char16*, Count);

 private:
  static const Count MIN_GAP_LENGTH = 1024;
  static const Count EXTENSION_LENGTH = 1024;

  void moveGap(Posn);

  base::char16* m_pwch;
  Count m_cwch;
  HANDLE m_hHeap;
  Posn m_lEnd;
  Posn m_lGapEnd;
  Posn m_lGapStart;
};

}  // namespace text

#endif  // EVITA_ED_BUFFERCORE_H_
