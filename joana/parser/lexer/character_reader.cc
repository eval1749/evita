// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/parser/lexer/character_reader.h"

#include "joana/public/source_code.h"
#include "joana/public/source_code_range.h"

namespace joana {
namespace internal {

CharacterReader::CharacterReader(const SourceCodeRange& range)
    : current_(range.start()), range_(range) {}

CharacterReader::~CharacterReader() = default;

const SourceCode& CharacterReader::source_code() const {
  return range_.source_code();
}

bool CharacterReader::HasMore() const {
  return unread_.has_value() || current_ < range_.end();
}

base::char16 CharacterReader::Read() {
  if (unread_.has_value()) {
    const auto value = unread_.value();
    unread_.reset();
    return value;
  }
  DCHECK_LT(current_, range_.end());
  const auto result = range_.source_code().GetString(current_, current_)[0];
  ++current_;
  return result;
}

void CharacterReader::Unread() {
  DCHECK(!unread_.has_value());
  DCHECK_GT(current_, range_.start());
  unread_.emplace(range_.source_code().GetString(current_ - 1, current_)[0]);
}

}  // namespace internal
}  // namespace joana
