// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/text/range_list.h"

#include "evita/text/buffer.h"
#include "evita/text/range.h"

namespace text {

RangeList::RangeList(Buffer* buffer) {
  buffer->AddObserver(this);
}

RangeList::~RangeList() {
  for (auto* range : ranges_) {
    range->m_pBuffer = nullptr;
  }
}

void RangeList::AddRange(Range* range) {
  ranges_.insert(range);
}

void RangeList::RemoveRange(Range* range) {
  ranges_.erase(range);
}

// BufferMutationObserver
void RangeList::DidDeleteAt(int offset, size_t length) {
  for (auto* range : ranges_) {
    if (range->m_lStart > offset)
      range->m_lStart -= length;
    if (range->m_lEnd > offset)
      range->m_lEnd -= length;
  }
}

void RangeList::DidInsertAt(int offset, size_t length) {
  for (auto* range : ranges_) {
    if (range->m_lStart > offset)
      range->m_lStart += length;
    if (range->m_lEnd > offset)
      range->m_lEnd += length;
  }
}

void RangeList::DidInsertBefore(int offset, size_t text_length) {
  for (auto* range : ranges_) {
    if (range->m_lStart >= offset)
      range->m_lStart += text_length;
    if (range->m_lEnd >= offset)
      range->m_lEnd += text_length;
  }
}

}  // namespace text
