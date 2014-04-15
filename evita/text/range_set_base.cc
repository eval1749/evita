// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/text/range_set_base.h"

#include "base/logging.h"
#include "evita/text/range_base.h"

namespace text {

//////////////////////////////////////////////////////////////////////
//
// RangeSetBase
//
RangeSetBase::RangeSetBase() {
}

RangeSetBase::~RangeSetBase() {
}

void RangeSetBase::set_range(RangeBase* range, Posn start, Posn end) {
  DCHECK_LE(start, end);
  DCHECK_GE(start, 0);
  range->start_ = start;
  range->end_ = end;
}

void RangeSetBase::set_range_end(RangeBase* range, Posn end) {
  DCHECK_LE(range->start_, end);
  range->end_ = end;
}

void RangeSetBase::set_range_start(RangeBase* range, Posn start) {
  DCHECK_LE(start, range->end_);
  DCHECK_GE(start, 0);
  range->start_ = start;
}

}  // namespace text
