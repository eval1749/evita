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

void CharacterReader::Advance() {
  DCHECK_LT(current_, range_.end());
  ++current_;
}

bool CharacterReader::AdvanceIf(base::char16 char_code) {
  if (!HasCharacter())
    return false;
  if (Get() != char_code)
    return false;
  Advance();
  return true;
}

base::char16 CharacterReader::Consume() {
  const auto result = Get();
  Advance();
  return result;
}

base::char16 CharacterReader::Get() const {
  DCHECK(HasCharacter());
  return range_.source_code().CharAt(current_);
}

bool CharacterReader::HasCharacter() const {
  return current_ < range_.end();
}

}  // namespace internal
}  // namespace joana
