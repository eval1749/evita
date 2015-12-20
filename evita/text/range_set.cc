// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/text/range_set.h"

#include <algorithm>

#include "evita/text/buffer.h"
#include "evita/text/range.h"
#include "evita/text/static_range.h"

namespace text {

//////////////////////////////////////////////////////////////////////
//
// RangeSet
//
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
void RangeSet::DidDeleteAt(const StaticRange& static_range) {
  const auto start = static_range.start();
  const auto end = static_range.end();
  const auto length = static_range.length();
  for (const auto& range : ranges_) {
    if (range->start_ > start)
      range->start_ = range->start_ >= end ? range->start_ - length : start;
    if (range->end_ > start)
      range->end_ = range->end_ >= end ? range->end_ - length : start;
  }
}

void RangeSet::DidInsertBefore(const StaticRange& static_range) {
  const auto offset = static_range.start();
  const auto length = static_range.length();
  for (const auto& range : ranges_) {
    if (range->start_ >= offset)
      range->start_ = range->start_ + length;
    if (range->end_ >= offset)
      range->end_ = range->end_ + length;
  }
}

}  // namespace text
