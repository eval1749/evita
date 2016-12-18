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

bool CharacterReader::CanPeekChar() const {
  return current_ < range_.end();
}

base::char16 CharacterReader::PeekChar() const {
  DCHECK(CanPeekChar());
  return range_.source_code().CharAt(current_);
}

base::char16 CharacterReader::ConsumeChar() {
  const auto result = PeekChar();
  Advance();
  return result;
}

bool CharacterReader::ConsumeCharIf(base::char16 char_code) {
  if (!CanPeekChar())
    return false;
  if (PeekChar() != char_code)
    return false;
  ConsumeChar();
  return true;
}

}  // namespace internal
}  // namespace joana
