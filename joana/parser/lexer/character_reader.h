// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_PARSER_LEXER_CHARACTER_READER_H_
#define JOANA_PARSER_LEXER_CHARACTER_READER_H_

#include "base/macros.h"
#include "base/strings/string16.h"

namespace joana {

class SourceCode;
class SourceCodeRange;

namespace internal {

class CharacterReader final {
 public:
  explicit CharacterReader(const SourceCodeRange& range);
  ~CharacterReader();

  int location() const { return current_; }
  const SourceCode& source_code() const;

  bool CanPeekChar() const;
  base::char16 ConsumeChar();

  // Returns true if |PeekChar()| is |expected_char| and advance to next
  // character.
  bool ConsumeCharIf(base::char16 expected_char);

  // For handling an RegExp starts with "=".
  void MoveBackward();

  base::char16 PeekChar() const;

 private:
  void MoveForward();

  int current_;
  const SourceCodeRange& range_;

  DISALLOW_COPY_AND_ASSIGN(CharacterReader);
};

}  // namespace internal
}  // namespace joana

#endif  // JOANA_PARSER_LEXER_CHARACTER_READER_H_
