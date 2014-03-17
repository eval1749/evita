// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/text/interval.h"

#include <algorithm>

#include "base/logging.h"
#include "evita/text/buffer.h"
#include "evita/text/interval_set.h"

#define DEBUG_INTERVAL _DEBUG

// Returns random number for Treap.
int TreapRandom() {
  DEFINE_STATIC_LOCAL(uint32, s_nRandom, (::GetTickCount()));
  s_nRandom = s_nRandom * 1664525 + 1013904223;
  return static_cast<int>(s_nRandom & ((1<<28)-1)) & MAXINT;
}

namespace text {

//////////////////////////////////////////////////////////////////////
//
// Interval
//
Interval::Interval(const Interval& other)
    : m_lEnd(other.m_lEnd),
      m_lStart(other.m_lStart),
      m_Style(other.m_Style) {
}

Interval::Interval(Posn lStart, Posn lEnd)
    : m_lEnd(lEnd), m_lStart(lStart) {
}

Interval::~Interval() {
}

// Returns true if this interval and pIntv can be merged. If two
// intervals are mergeable, both intervals have equivalent styles.
bool Interval::CanMerge(const Interval* pIntv) const {
  return GetStyle() == pIntv->GetStyle();
}

void Interval::SetStyle(const css::Style& other) {
  m_Style.OverrideBy(other);
}

//////////////////////////////////////////////////////////////////////
//
// Buffer
//
Interval* Buffer::GetIntervalAt(Posn lPosn) const {
  return intervals_->GetIntervalAt(std::min(lPosn, GetEnd()));
}

void Buffer::SetStyle(Posn lStart, Posn lEnd, const css::Style& style) {
  if (lStart < 0)
    lStart = 0;
  if (lEnd > GetEnd())
    lEnd = GetEnd();
  if (lStart == lEnd)
    return;
  DCHECK_LT(lStart, lEnd);
  // To improve performance, we don't check contents of |style|.
  // This may be enough for syntax coloring.
  m_nModfTick += 1;
  SetStyleInternal(lStart, lEnd, style);
  FOR_EACH_OBSERVER(BufferMutationObserver, observers_,
    DidChangeStyle(lStart, static_cast<size_t>(lEnd - lStart)));
}

void Buffer::SetStyleInternal(Posn start, Posn end, const css::Style& style) {
  DCHECK_LT(start, end);
  auto offset = start;
  while (offset < end) {
    auto const interval = GetIntervalAt(offset);
    DCHECK_LE(interval->GetStart(), offset);
    auto const target = interval->GetStart() == offset ? interval :
        intervals_->SplitAt(interval, offset);
    if (target->GetEnd() == end) {
      target->SetStyle(style);
      tryMergeInterval(target);
      break;
    }

    if (target->GetEnd() > end) {
      intervals_->SplitAt(target, end);
      target->SetStyle(style);
      tryMergeInterval(target);
      break;
    }

    target->SetStyle(style);
    offset = tryMergeInterval(target)->GetEnd();
  }
}

Interval* Buffer::tryMergeInterval(Interval* pIntv) {
  // Merge to previous
  if (Interval* pPrev = pIntv->GetPrev()) {
    DCHECK_EQ(pPrev->GetEnd(), pIntv->GetStart());
    DCHECK_EQ(pPrev->GetNext(), pIntv);

    if (pIntv->CanMerge(pPrev)) {
        auto const lEnd = pIntv->GetEnd();
        intervals_->RemoveInterval(pIntv);
        delete pIntv;
        pIntv = pPrev;
        pIntv->m_lEnd = lEnd;
    }
  }

  // Absobe next
  if (Interval* pNext = pIntv->GetNext()) {
    DCHECK_EQ(pIntv->GetEnd(), pNext->GetStart());
    if (pIntv->CanMerge(pNext)) {
      auto const lEnd = pNext->GetEnd();
      intervals_->RemoveInterval(pNext);
      delete pNext;
      pIntv->m_lEnd = lEnd;
    }
  }

  return pIntv;
}

}  // namespace text
