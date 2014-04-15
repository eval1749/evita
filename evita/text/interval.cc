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

void Interval::SetStyle(const css::Style& other) {
  m_Style.OverrideBy(other);
}

}  // namespace text
