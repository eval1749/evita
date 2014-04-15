// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/text/interval.h"

#include "base/logging.h"
#include "evita/text/buffer.h"

namespace text {

//////////////////////////////////////////////////////////////////////
//
// Interval
//
Interval::Interval(const Interval& other)
    : RangeBase(other), style_(other.style_) {
  DCHECK_LT(start(), end());
}

Interval::Interval(Posn start, Posn end) : RangeBase(start, end) {
  DCHECK_LT(start, end);
}

Interval::~Interval() {
}

void Interval::set_style(const css::Style& style) {
  style_.OverrideBy(style);
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
