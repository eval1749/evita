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
    : end_(other.end_),
      start_(other.start_),
      style_(other.style_) {
}

Interval::Interval(Posn start, Posn end)
    : end_(end), start_(start) {
}

Interval::~Interval() {
}

void Interval::set_style(const css::Style& style) {
  style_.OverrideBy(style);
}

int Interval::Compare(const Interval* other) const {
  return start_ - other->start_;
}

bool Interval::Contains(Posn offset) const {
  return offset >= start_ && offset < end_;
}

}  // namespace text

namespace std {
ostream& operator<<(ostream& ostream, const text::Interval& interval) {
  return ostream << "text::Interval(" << interval.start() << ", " <<
      interval.end() << ")";
}

ostream& operator<<(ostream& ostream, const text::Interval* interval) {
  if (!interval)
    return ostream << "NullInterval";
  return ostream << *interval;
}
}  // namespace std
