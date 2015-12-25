// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_ISTRINGCURSOR_H_
#define EVITA_ISTRINGCURSOR_H_

#include "base/strings/string16.h"
#include "evita/text/offset.h"

struct IStringCursor {
  virtual bool CanMove(int count) = 0;

  virtual base::char16 GetChar(int offset) = 0;
  virtual text::Offset GetPosition() = 0;

  // TODO(eval1749): We should use |int| for |FindBackward()|
  virtual bool FindBackward(base::char16 ch, uint32_t count) = 0;
  // TODO(eval1749): We should use |int| for |FindForward()|
  virtual bool FindForward(base::char16 ch, uint32_t count) = 0;
  // TODO(eval1749): We should use |int| for |Match()|
  virtual bool Match(const base::char16* sting, int offset, uint32_t count) = 0;
  virtual text::Offset Move(int count) = 0;
  virtual text::Offset MoveToEnd() = 0;

  virtual text::Offset MoveToStart() = 0;
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
