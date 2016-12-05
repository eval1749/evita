// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_PARSER_LEXER_CHARACTER_READER_H_
#define JOANA_PARSER_LEXER_CHARACTER_READER_H_

#include "base/macros.h"
#include "base/optional.h"
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

  bool HasMore() const;
  base::char16 Read();
  void Unread();

 private:
  int current_;
  const SourceCodeRange& range_;
  base::Optional<base::char16> unread_;

  DISALLOW_COPY_AND_ASSIGN(CharacterReader);
};

}  // namespace internal
}  // namespace joana

#endif  // JOANA_PARSER_LEXER_CHARACTER_READER_H_
