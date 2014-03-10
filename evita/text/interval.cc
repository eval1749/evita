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
Interval::Interval(Posn lStart, Posn lEnd, int nZ)
    : m_lEnd(lEnd),
      m_lStart(lStart),
      m_nZ(nZ),
      m_Style(*css::Style::Default()) {
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
  if (lStart >= lEnd)
    return;

  // To improve performance, we don't check contents of |style|.
  // This may be enough for syntax coloring.
  m_nModfTick += 1;

  // Get interval pHead containing lStart.
  auto const pHead = GetIntervalAt(lStart);

  auto const lHeadEnd = pHead->GetEnd();
  auto const lHeadStart = pHead->GetStart();

  if (lHeadStart == lStart && lHeadEnd == lEnd) {
    // pHead: ---s......e---
    // Range: ---s......e---
    pHead->SetStyle(style);
    tryMergeInterval(pHead);
    return;
  }

  if (lHeadEnd < lEnd) {
    // pHead: --s...e----
    // Range: ----s.....e----
    SetStyle(lStart, lHeadEnd, style);
    SetStyle(lHeadEnd, lEnd, style);
    return;
  }

  // New style is compatibile with existing one.
  Interval oIntv(lStart, lEnd);
  oIntv.SetStyle(style);
  if (oIntv.CanMerge(pHead))
    return;

  if (lHeadStart == lStart) {
    // pHead: ---s........e---
    // pTail: --------s...e---
    // Range: ---s....e-------
    pHead->m_lStart = lEnd;
    if (auto const pPrev = pHead->GetPrev()) {
      if (oIntv.CanMerge(pPrev)) {
        pPrev->m_lEnd = lEnd;
        return;
      }
    }

    auto const pIntv = new Interval(lStart, lEnd);
    pIntv->SetStyle(style);

    intervals_->InsertBefore(pIntv, pHead);

    DCHECK_EQ(pHead->GetPrev(), pIntv);
    DCHECK_EQ(pIntv->GetNext(), pHead);
    #if DEBUG_INTERVAL
      DCHECK_EQ(pIntv, GetIntervalAt(pIntv->GetStart()));
    #endif
    return;
  }

  if (lHeadEnd == lEnd) {
    // pHead: ---s........e---
    // Range: -------s....e---
    pHead->m_lEnd = lStart;

    if (auto const pNext = pHead->GetNext()) {
      if (oIntv.CanMerge(pNext)) {
        pNext->m_lStart = lStart;
        return;
      }
    }

    auto const pIntv = new Interval(lStart, lEnd);
    pIntv->SetStyle(style);
    intervals_->InsertAfter(pIntv, pHead);
    DCHECK_EQ(pHead->GetNext(), pIntv);
    DCHECK_EQ(pIntv->GetPrev(), pHead);
    #if DEBUG_INTERVAL
      DCHECK_EQ(pIntv, GetIntervalAt(pIntv->GetStart()));
    #endif
    return;
  }

  // pHead: ---s...........e---
  // pTail: ----------s....e---
  // Range: -----s....e--------
  pHead->m_lEnd = lStart;

  auto const pTail = new Interval(lEnd, lHeadEnd);
  pTail->SetStyle(pHead->GetStyle());
  intervals_->InsertAfter(pTail, pHead);
  DCHECK_EQ(pHead->GetNext(), pTail);
  DCHECK_EQ(pTail->GetPrev(), pHead);
  #if DEBUG_INTERVAL
    DCHECK_EQ(pTail, GetIntervalAt(pTail->GetStart()));
  #endif

  auto const pIntv = new Interval(lStart, lEnd);
  pIntv->SetStyle(style);
  intervals_->InsertAfter(pIntv, pHead);
  DCHECK_EQ(pHead->GetNext(), pIntv);
  DCHECK_EQ(pIntv->GetPrev(), pHead);
  #if DEBUG_INTERVAL
    DCHECK_EQ(pIntv, GetIntervalAt(pIntv->GetStart()));
  #endif
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
