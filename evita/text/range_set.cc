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
    range->m_pBuffer = nullptr;
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
    if (range->m_lStart > offset) {
      range->m_lStart = std::max(static_cast<Posn>(range->m_lStart - length),
                                 offset);
    }
    if (range->m_lEnd > offset) {
      range->m_lEnd = std::max(static_cast<Posn>(range->m_lEnd - length),
                               offset);
    }
  }
}

void RangeSet::DidInsertAt(Posn offset, size_t length) {
  for (auto* range : ranges_) {
    if (range->m_lStart > offset)
      range->m_lStart += length;
    if (range->m_lEnd > offset)
      range->m_lEnd += length;
  }
}

void RangeSet::DidInsertBefore(Posn offset, size_t length) {
  for (auto* range : ranges_) {
    if (range->m_lStart >= offset)
      range->m_lStart += length;
    if (range->m_lEnd >= offset)
      range->m_lEnd += length;
  }
}

}  // namespace text
