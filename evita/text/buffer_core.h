// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_TEXT_BUFFER_CORE_H_
#define EVITA_TEXT_BUFFER_CORE_H_

// TOOD(eval1749): We should not include "windows.h" here.
#include <windows.h>

#include "base/strings/string16.h"
#include "evita/text/offset.h"

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
  Offset EnsurePosn(int offset) const;

  // [G]
  base::char16 GetCharAt(Offset) const;
  Offset GetEnd() const { return m_lEnd; }
  OffsetDelta GetText(base::char16*, Offset, Offset) const;
  base::string16 GetText(Offset start, Offset end) const;

  // [I]
  bool IsValidPosn(Offset p) const { return p >= 0 && p <= m_lEnd; }

  bool IsValidRange(Offset s, Offset e) const {
    return IsValidPosn(s) && IsValidPosn(e) && s <= e;
  }

 protected:
  BufferCore();

  OffsetDelta deleteChars(Offset from, Offset to);
  void extend(Offset from, int);
  void insert(Offset offset, const base::char16* chars, size_t length);

 private:
  void moveGap(Offset offset);

  base::char16* m_pwch;
  int m_cwch;
  HANDLE m_hHeap;
  Offset m_lEnd;
  Offset m_lGapEnd;
  Offset m_lGapStart;
};

}  // namespace text

#endif  // EVITA_TEXT_BUFFER_CORE_H_
