// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/parser/lexer/character_reader.h"

#include "joana/public/source_code.h"
#include "joana/public/source_code_range.h"

namespace joana {
namespace internal {

CharacterReader::CharacterReader(const SourceCodeRange& range)
    : current_char_offset_(range.start()), range_(range) {
  FetchChar();
}

CharacterReader::~CharacterReader() = default;

int CharacterReader::location() const {
  return current_char_offset_;
}

const SourceCode& CharacterReader::source_code() const {
  return range_.source_code();
}

bool CharacterReader::CanPeekChar() const {
  return current_char_ >= 0;
}

base::char16 CharacterReader::PeekChar() const {
  DCHECK(CanPeekChar());
  return current_char_;
}

base::char16 CharacterReader::ConsumeChar() {
  const auto result = PeekChar();
  MoveForward();
  return result;
}

bool CharacterReader::ConsumeCharIf(base::char16 char_code) {
  if (!CanPeekChar() || PeekChar() != char_code)
    return false;
  ConsumeChar();
  return true;
}

void CharacterReader::FetchChar() {
  if (current_char_offset_ == range_.end()) {
    current_char_ = -1;
    return;
  }
  current_char_ = source_code().CharAt(current_char_offset_);
}

void CharacterReader::MoveBackward() {
  DCHECK_GT(current_char_offset_, range_.start());
  --current_char_offset_;
  FetchChar();
}

void CharacterReader::MoveForward() {
  DCHECK_LT(current_char_offset_, range_.end());
  ++current_char_offset_;
  FetchChar();
}

}  // namespace internal
}  // namespace joana
