// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/text/range_base.h"

#include "base/logging.h"

namespace text {
//////////////////////////////////////////////////////////////////////
//
// RangeBase
//
RangeBase::RangeBase(Offset start, Offset end) : end_(end), start_(start) {
  DCHECK_LE(start_, end_);
  DCHECK_GE(start_, Offset(0));
}

RangeBase::RangeBase(const RangeBase& other)
    : end_(other.end_), start_(other.start_) {}

RangeBase::~RangeBase() {}

bool RangeBase::Contains(Offset offset) const {
  return offset >= start() && offset < end();
}

}  // namespace text
