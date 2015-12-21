// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>

#include "evita/text/buffer_core.h"

#include "base/logging.h"
#include "evita/text/offset.h"

namespace text {

const int MIN_GAP_LENGTH = 1024;
const int EXTENSION_LENGTH = 1024;

BufferCore::BufferCore()
    : m_cwch(MIN_GAP_LENGTH * 3), m_lEnd(0), m_lGapEnd(m_cwch), m_lGapStart(0) {
  m_hHeap = ::HeapCreate(HEAP_NO_SERIALIZE, 0, 0);
  DCHECK(m_hHeap);
  m_pwch = reinterpret_cast<base::char16*>(
      ::HeapAlloc(m_hHeap, 0, sizeof(base::char16) * m_cwch));
}

BufferCore::~BufferCore() {
  if (m_hHeap)
    ::HeapDestroy(m_hHeap);
}

OffsetDelta BufferCore::deleteChars(Offset lStart, Offset lEnd) {
  DCHECK(IsValidRange(lStart, lEnd));
  auto const n = lEnd - lStart;
  moveGap(lStart);
  m_lGapEnd += n;
  m_lEnd -= n;
  return n;
}

Offset BufferCore::EnsurePosn(int offset) const {
  if (offset < 0)
    return Offset(0);
  if (offset > m_lEnd.value())
    return m_lEnd;
  return Offset(offset);
}

void BufferCore::extend(Offset lPosn, int cwchExtent) {
  if (cwchExtent <= 0)
    return;

  moveGap(lPosn);

  if ((m_lGapEnd - m_lGapStart) >= cwchExtent + MIN_GAP_LENGTH) {
    // We have enough GAP.
    return;
  }

  auto nExtension = cwchExtent + EXTENSION_LENGTH - 1;
  nExtension /= EXTENSION_LENGTH;
  nExtension *= EXTENSION_LENGTH;
  m_cwch += nExtension;

  // Extend character buffer
  m_pwch = static_cast<base::char16*>(
      ::HeapReAlloc(m_hHeap, 0, m_pwch, sizeof(base::char16) * m_cwch));
  DCHECK(m_pwch);
  // Extend GAP
  ::MoveMemory(m_pwch + m_lGapEnd.value() + nExtension,
               m_pwch + m_lGapEnd.value(),
               sizeof(base::char16) * (m_lEnd - m_lGapStart));

  m_lGapEnd += OffsetDelta(nExtension);
}

base::char16 BufferCore::GetCharAt(Offset lPosn) const {
  DCHECK(IsValidPosn(lPosn));
  if (lPosn >= GetEnd())
    return 0;
  if (lPosn >= m_lGapStart)
    lPosn += m_lGapEnd - m_lGapStart;
  return m_pwch[lPosn.value()];
}

OffsetDelta BufferCore::GetText(base::char16* prgwch,
                                Offset lStart,
                                Offset lEnd) const {
  if (lEnd > GetEnd())
    lEnd = GetEnd();
  if (lStart >= lEnd)
    return OffsetDelta(0);

  if (lStart >= m_lGapStart) {
    // We extract text after gap.
    // gggggg<....>
    ::CopyMemory(prgwch, m_pwch + m_lGapEnd.value() + (lStart - m_lGapStart),
                 sizeof(base::char16) * (lEnd - lStart));
  } else {
    // We extract text before gap.
    // <.....>gggg
    // <...ggg>ggg
    // <...ggg...>
    auto const lMiddle = std::min(m_lGapStart, lEnd);
    ::CopyMemory(prgwch, m_pwch + lStart.value(),
                 sizeof(base::char16) * (lMiddle - lStart));
    ::CopyMemory(prgwch + (lMiddle - lStart), m_pwch + m_lGapEnd.value(),
                 sizeof(base::char16) * (lEnd - lMiddle));
  }

  return lEnd - lStart;
}

base::string16 BufferCore::GetText(Offset start, Offset end) const {
  if (start > GetEnd())
    start = GetEnd();
  if (end > GetEnd())
    end = GetEnd();
  auto const cwch = end - start;
  if (cwch <= 0)
    return base::string16();
  base::string16 text(static_cast<size_t>(cwch.value()), ' ');
  GetText(&text[0], start, end);
  return std::move(text);
}

// Inserts specified string (pwch, n) before lPosn.
void BufferCore::insert(Offset lPosn, const base::char16* pwch, size_t n) {
  DCHECK(IsValidPosn(lPosn));
  extend(lPosn, n);
  ::CopyMemory(m_pwch + lPosn.value(), pwch, sizeof(base::char16) * n);
  m_lGapStart += OffsetDelta(n);
  m_lEnd += OffsetDelta(n);
}

// User 1 2 3 4 5 6 7 8           9 A B
//       M i n n e a p o _ _ _ _ _ l i s
// Gap  1 2 3 4 5 6 7 8 9 A B C D E F 10
//              Offset     Gap
void BufferCore::moveGap(Offset lNewStart) {
  auto const lCurEnd = m_lGapEnd;
  auto const lCurStart = m_lGapStart;
  auto const iDiff = m_lGapStart - lNewStart;
  auto const lNewEnd = m_lGapEnd - iDiff;
  m_lGapEnd = lNewEnd;
  m_lGapStart = lNewStart;

  if (iDiff > 0) {
    // Move GAP backward
    //  Move GAP between lNewStart and lCurStart before lCurEnd.
    // abcdef....ghijk
    //    ^  s   e
    // abc....defghijk
    //    s   e
    ::MoveMemory(m_pwch + lNewEnd.value(), m_pwch + lNewStart.value(),
                 sizeof(base::char16) * iDiff);
  } else if (iDiff < 0) {
    // Move GAP forward
    //  Move string between lCurEnd and m_lGapEnd after lCurStart.
    // abcde...fghijk
    //      s  e   ^
    //         |   |
    //      +--+   |
    //      |      |
    //      |   +--+
    //      V   V
    // abcdefghi...jk
    //          s  e
    ::MoveMemory(m_pwch + lCurStart.value(), m_pwch + lCurEnd.value(),
                 sizeof(base::char16) * -iDiff);
  }
}

}  // namespace text
