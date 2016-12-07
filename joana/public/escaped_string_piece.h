// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_PUBLIC_ESCAPED_STRING_PIECE_H_
#define JOANA_PUBLIC_ESCAPED_STRING_PIECE_H_

#include <ostream>
#include <string>

#include "base/macros.h"
#include "base/strings/string16.h"
#include "base/strings/string_piece.h"
#include "joana/public/public_export.h"

namespace joana {

class JOANA_PUBLIC_EXPORT EscapedStringPiece16 {
 public:
  EscapedStringPiece16(base::StringPiece16 piece, char delimiter);
  ~EscapedStringPiece16();

  char delimiter() const { return delimiter_; }
  base::StringPiece16 string() const { return string_; }

 private:
  const char delimiter_;
  const base::StringPiece16 string_;

  DISALLOW_COPY_AND_ASSIGN(EscapedStringPiece16);
};

std::ostream& operator<<(std::ostream& ostream,
                         const EscapedStringPiece16& escaped);

}  // namespace joana

#endif  // JOANA_PUBLIC_ESCAPED_STRING_PIECE_H_