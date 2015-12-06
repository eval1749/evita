// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>

#include "evita/text/buffer_core.h"

#include "base/logging.h"

namespace text {

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

Count BufferCore::deleteChars(Posn lStart, Posn lEnd) {
  DCHECK(IsValidRange(lStart, lEnd));
  auto const n = lEnd - lStart;
  moveGap(lStart);
  m_lGapEnd += n;
  m_lEnd -= n;
  return n;
}

void BufferCore::extend(Posn lPosn, int cwchExtent) {
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
  ::MoveMemory(m_pwch + m_lGapEnd + nExtension, m_pwch + m_lGapEnd,
               sizeof(base::char16) * (m_lEnd - m_lGapStart));

  m_lGapEnd += nExtension;
}

base::char16 BufferCore::GetCharAt(Posn lPosn) const {
  DCHECK(IsValidPosn(lPosn));
  if (lPosn >= GetEnd())
    return 0;
  if (lPosn >= m_lGapStart)
    lPosn += m_lGapEnd - m_lGapStart;
  return m_pwch[lPosn];
}

Count BufferCore::GetText(base::char16* prgwch, Posn lStart, Posn lEnd) const {
  if (lStart < 0)
    lStart = 0;
  if (lEnd > GetEnd())
    lEnd = GetEnd();
  if (lStart >= lEnd)
    return 0;

  if (lStart >= m_lGapStart) {
    // We extract text after gap.
    // gggggg<....>
    ::CopyMemory(prgwch, m_pwch + m_lGapEnd + (lStart - m_lGapStart),
                 sizeof(base::char16) * (lEnd - lStart));
  } else {
    // We extract text before gap.
    // <.....>gggg
    // <...ggg>ggg
    // <...ggg...>
    auto const lMiddle = std::min(m_lGapStart, lEnd);
    ::CopyMemory(prgwch, m_pwch + lStart,
                 sizeof(base::char16) * (lMiddle - lStart));
    ::CopyMemory(prgwch + (lMiddle - lStart), m_pwch + m_lGapEnd,
                 sizeof(base::char16) * (lEnd - lMiddle));
  }

  return lEnd - lStart;
}

base::string16 BufferCore::GetText(Posn start, Posn end) const {
  if (start < 0)
    start = std::max(GetEnd() + start, static_cast<Posn>(0));
  else if (start > GetEnd())
    start = GetEnd();
  if (end < 0)
    end = std::max(GetEnd() + end, static_cast<Posn>(0));
  else if (end > GetEnd())
    end = GetEnd();
  auto const cwch = end - start;
  if (cwch <= 0)
    return base::string16();
  base::string16 text(static_cast<size_t>(cwch), ' ');
  GetText(&text[0], start, end);
  return std::move(text);
}

// Inserts specified string (pwch, n) before lPosn.
void BufferCore::insert(Posn lPosn, const base::char16* pwch, Count n) {
  DCHECK(IsValidPosn(lPosn));
  extend(lPosn, n);
  ::CopyMemory(m_pwch + lPosn, pwch, sizeof(base::char16) * n);
  m_lGapStart += n;
  m_lEnd += n;
}

// User 1 2 3 4 5 6 7 8           9 A B
//       M i n n e a p o _ _ _ _ _ l i s
// Gap  1 2 3 4 5 6 7 8 9 A B C D E F 10
//              Posn     Gap
void BufferCore::moveGap(Posn lNewStart) {
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
    ::MoveMemory(m_pwch + lNewEnd, m_pwch + lNewStart,
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
    ::MoveMemory(m_pwch + lCurStart, m_pwch + lCurEnd,
                 sizeof(base::char16) * -iDiff);
  }
}

}  // namespace text
