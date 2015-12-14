// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/text/range_set.h"

#include <algorithm>

#include "evita/text/buffer.h"
#include "evita/text/range.h"

namespace text {

RangeSet::RangeSet(Buffer* buffer) {
  buffer->AddObserver(this);
}

RangeSet::~RangeSet() {
  for (auto* range : ranges_) {
    range->buffer_ = nullptr;
  }
}

void RangeSet::AddRange(Range* range) {
  ranges_.insert(range);
}

void RangeSet::RemoveRange(Range* range) {
  ranges_.erase(range);
}

// BufferMutationObserver
void RangeSet::DidDeleteAt(Offset start, OffsetDelta length) {
  auto const end = start + length;
  for (auto* range : ranges_) {
    if (range->start_ > start)
      range->start_ = range->start_ >= end ? range->start_ - length : start;
    if (range->end_ > start)
      range->end_ = range->end_ >= end ? range->end_ - length : start;
  }
}

void RangeSet::DidInsertBefore(Offset offset, OffsetDelta length) {
  for (auto* range : ranges_) {
    if (range->start_ >= offset)
      range->start_ = range->start_ + length;
    if (range->end_ >= offset)
      range->end_ = range->end_ + length;
  }
}

}  // namespace text
