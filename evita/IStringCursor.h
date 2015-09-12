//////////////////////////////////////////////////////////////////////////////
//
// evcl - listener - edit buffer
// listener/winapp/ed_buffer.h
//
// Copyright (C) 1996-2007 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evcl3/mainline/listener/winapp/IStringCursor.h#1 $
//
#ifndef EVITA_ISTRINGCURSOR_H_
#define EVITA_ISTRINGCURSOR_H_

#include "base/basictypes.h"
#include "base/strings/string16.h"
#include "evita/precomp.h"

struct IStringCursor {
  virtual bool CanMove(int count) = 0;

  virtual char16 GetChar(int offset) = 0;
  virtual text::Posn GetPosition() = 0;

  // TODO(eval1749) We should use |int| for |FindBackward()|
  virtual bool FindBackward(base::char16 ch, uint32_t count) = 0;
  // TODO(eval1749) We should use |int| for |FindForward()|
  virtual bool FindForward(base::char16 ch, uint32_t count) = 0;
  // TODO(eval1749) We should use |int| for |Match()|
  virtual bool Match(const base::char16* sting, int offset, uint32_t count) = 0;
  virtual text::Posn Move(int count) = 0;
  virtual text::Posn MoveToEnd() = 0;

  virtual text::Posn MoveToStart() = 0;
};

struct StringRange {
  int m_lStart;
  int m_lEnd;
};

struct IStringMatcher {
  virtual bool FirstMatch(IStringCursor* cursor) = 0;
  virtual bool GetMatched(int offset, StringRange* range) = 0;
  virtual bool NextMatch() = 0;
};

#endif  // EVITA_ISTRINGCURSOR_H_
