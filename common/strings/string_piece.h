// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_common_strings_string_piece_h)
#define INCLUDE_common_strings_string_piece_h

#include "base/strings/string_piece.h"

namespace std {
template<>
struct hash<base::StringPiece> {
  std::size_t operator()(const base::StringPiece& sp) const {
    HASH_STRING_PIECE(base::StringPiece, sp);
  }
};
}  // namespace std

#endif //!defined(INCLUDE_common_strings_string_piece_h)
