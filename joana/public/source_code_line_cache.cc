// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>
#include <iterator>

#include "joana/public/source_code_line_cache.h"

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

SourceCodeLine::Cache::Cache(const SourceCode& source_code)
    : source_code_(source_code) {}

SourceCodeLine::Cache::~Cache() = default;

SourceCodeLine SourceCodeLine::Cache::Get(int offset) const {
  DCHECK_GE(offset, 0);
  if (offset > runner_ || offsets_.empty()) {
    // Extend offset cache until |offset|.
    while (runner_ < source_code_.size()) {
      ++runner_;
      if (!IsLineTerminator(source_code_.GetChar(runner_ - 1)))
        continue;
      offsets_.push_back(runner_);
      if (runner_ > offset)
        break;
    }
  }
  DCHECK_LE(offset, runner_);

  if (offsets_.empty()) {
    // There is no line terminate in |source_code_|.
    return SourceCodeLine(source_code_.range(), 1);
  }

  if (offset < offsets_.front()) {
    // |offset| is in the first line.
    return SourceCodeLine(source_code_.Slice(0, offsets_.front()), 1);
  }

  const auto& begin = offsets_.begin();
  const auto& end = offsets_.end();
  const auto& it = std::lower_bound(begin, end, offset);
  DCHECK(it != end);
  const auto& start = *it == offset ? it : std::prev(it);
  const auto line_number = static_cast<int>(start - begin) + 2;
  const auto& next = std::next(start);
  if (next == end) {
    return SourceCodeLine(source_code_.Slice(*start, source_code_.size()),
                          line_number);
  }
  return SourceCodeLine(source_code_.Slice(*start, *next), line_number);
}

}  // namespace joana
