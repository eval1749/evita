// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>
#include <iterator>

#include "joana/public/line_number_cache.h"

#include "base/logging.h"
#include "joana/public/source_code.h"

namespace joana {

using ColumnNumber = LineNumberCache::ColumnNumber;
using LineNumber = LineNumberCache::LineNumber;
using Offset = LineNumberCache::Offset;

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
    : source_code_(source_code) {}

LineNumberCache::~LineNumberCache() = default;

std::pair<int, int> LineNumberCache::Get(int offset) const {
  DCHECK_GE(offset, 0);
  if (offset == 0)
    return std::make_pair(1, 0);
  if (offset > last_offset_) {
    // Extend offset cache until |offset|.
    for (auto runner = last_offset_; runner < offset; ++runner) {
      if (!IsEndOfLine(runner))
        continue;
      line_start_offsets_.push_back(runner + 1);
    }
    last_offset_ = offset;
  }

  if (line_start_offsets_.empty() || offset < line_start_offsets_.front()) {
    // |offset| is still in the first line.
    return std::make_pair(1, offset);
  }

  if (offset >= line_start_offsets_.back()) {
    // |offset| is in the last scanned line.
    const auto line_number =
        static_cast<LineNumber>(line_start_offsets_.size()) + 1;
    const auto column_number = offset - line_start_offsets_.back();
    return std::make_pair(line_number, column_number);
  }

  const auto& begin = line_start_offsets_.begin();
  const auto& it = std::lower_bound(begin, line_start_offsets_.end(), offset);
  const auto& start = *it == offset ? it : std::prev(it);
  const auto line_number = static_cast<LineNumber>(start - begin) + 2;
  const auto column_number = offset - *start;
  return std::make_pair(line_number, column_number);
}

bool LineNumberCache::IsEndOfLine(Offset offset) const {
  DCHECK_LE(offset, source_code_.size());
  return offset == source_code_.size() ||
         IsLineTerminator(source_code_.CharAt(offset));
}

}  // namespace joana
