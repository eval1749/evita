// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <iterator>

#include "joana/public/line_number_cache.h"

#include "base/logging.h"
#include "joana/public/source_code.h"

namespace joana {

namespace {

const int kMaxUnicodeCodePoint = 0x10FFFF;
const base::char16 kLineSeparator = 0x2028;
const base::char16 kParagraphSeparator = 0x2029;

bool IsLineTerminator(base::char16 char_code) {
  return char_code == '\n' || char_code == '\r' ||
         char_code == kLineSeparator || char_code == kParagraphSeparator;
}

}  // namespace

LineNumberCache::LineNumberCache(const SourceCode& source_code)
    : source_code_(source_code) {
  map_.emplace(0, 1);
}

LineNumberCache::~LineNumberCache() = default;

std::pair<int, int> LineNumberCache::Get(int offset) const {
  DCHECK_GE(offset, 0);
  if (offset == 0)
    return std::make_pair(1, 0);
  const auto& it = map_.lower_bound(offset);
  if (it == map_.end()) {
    // offset is after cache.
    return UpdateCache(offset);
  }

  if (it->first == offset)
    return std::make_pair(it->second, it->first);

  DCHECK(it != map_.begin());
  const auto& previous = std::prev(it);
  DCHECK_GT(offset, previous->first);
  return std::make_pair(previous->second, previous->first);
}

bool LineNumberCache::IsEndOfLine(Offset offset) const {
  DCHECK_LE(offset, source_code_.size());
  return offset == source_code_.size() ||
         IsLineTerminator(source_code_.GetChar(offset));
}

std::pair<int, int> LineNumberCache::UpdateCache(Offset offset) const {
  DCHECK_LE(offset, source_code_.size());
  DCHECK(!map_.empty());
  auto line_number = map_.rbegin()->second;
  auto line_start = map_.rbegin()->first;
  DCHECK_LT(line_start, offset);
  for (auto runner = line_start; runner < offset; ++runner) {
    if (!IsEndOfLine(runner))
      continue;
    ++line_number;
    line_start = runner + 1;
    const auto& result = map_.emplace(line_start, line_number);
    DCHECK(result.second) << line_start << " must be inserted.";
  }
  return std::make_pair(line_number, line_start);
}

}  // namespace joana
