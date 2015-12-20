// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/text/line_number_cache.h"

#include "evita/text/buffer.h"
#include "evita/text/static_range.h"

namespace text {

namespace {
LineNumberAndOffset MakeLineNumberAndOffset(Offset offset, int line_number) {
  LineNumberAndOffset result;
  result.number = line_number;
  result.offset = offset;
  return result;
}
}  // namespace

LineNumberCache::LineNumberCache(const Buffer& buffer) : buffer_(buffer) {
  const_cast<Buffer&>(buffer_).AddObserver(this);
  map_[Offset()] = 1;
}

LineNumberCache::~LineNumberCache() {
  const_cast<Buffer&>(buffer_).RemoveObserver(this);
}

LineNumberAndOffset LineNumberCache::Get(Offset offset) {
  DCHECK(!map_.empty());
  if (!offset)
    return MakeLineNumberAndOffset(Offset(), Offset(1));
  auto it = map_.lower_bound(offset);
  if (it == map_.end()) {
    // offset is after cache.
    auto const last = map_.rbegin();
    return UpdateCache(last->first, last->second, offset);
  }
  if (it->first == offset)
    return MakeLineNumberAndOffset(it->first, it->second);
  DCHECK(it != map_.begin());
  --it;
  DCHECK_GT(offset, it->first);
  return MakeLineNumberAndOffset(it->first, it->second);
}

void LineNumberCache::InvalidateCache(Offset offset) {
  if (!offset) {
    map_.clear();
    map_[Offset(0)] = 1;
    return;
  }
  for (;;) {
    auto it = map_.lower_bound(offset);
    if (it == map_.end())
      break;
    map_.erase(it);
  }
}

LineNumberAndOffset LineNumberCache::UpdateCache(Offset line_start_offset,
                                                 int line_number,
                                                 Offset goal_offset) {
  DCHECK(!map_.empty());
  auto start_offset = line_start_offset;
  for (auto offset = line_start_offset; offset < goal_offset; ++offset) {
    if (buffer_.GetCharAt(offset) == 0x0A) {
      ++line_number;
      start_offset = offset + OffsetDelta(1);
      map_[start_offset] = line_number;
    }
  }
  return MakeLineNumberAndOffset(start_offset, line_number);
}

// BufferMutationObserver
void LineNumberCache::DidDeleteAt(const StaticRange& range) {
  InvalidateCache(range.start());
}

void LineNumberCache::DidInsertBefore(const StaticRange& range) {
  InvalidateCache(range.start());
}

}  // namespace text
