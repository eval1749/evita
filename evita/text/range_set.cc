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
void RangeSet::DidDeleteAt(Posn offset, size_t length) {
  for (auto* range : ranges_) {
    if (range->start_ > offset) {
      range->start_ =
          std::max(static_cast<Posn>(range->start_ - length), offset);
    }
    if (range->end_ > offset) {
      range->end_ = std::max(static_cast<Posn>(range->end_ - length), offset);
    }
  }
}

void RangeSet::DidInsertAt(Posn offset, size_t length) {
  for (auto* range : ranges_) {
    if (range->start_ > offset)
      range->start_ = static_cast<Posn>(range->start_ + length);
    if (range->end_ > offset)
      range->end_ = static_cast<Posn>(range->end_ + length);
  }
}

void RangeSet::DidInsertBefore(Posn offset, size_t length) {
  for (auto* range : ranges_) {
    if (range->start_ >= offset)
      range->start_ = static_cast<Posn>(range->start_ + length);
    if (range->end_ >= offset)
      range->end_ = static_cast<Posn>(range->end_ + length);
  }
}

}  // namespace text
