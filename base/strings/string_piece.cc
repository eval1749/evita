// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "base/strings/string_piece.h"

namespace base {

BASE_EXPORT bool operator==(const StringPiece& piece1,
                            const StringPiece& piece2) {
  if (piece1.data() == piece2.data())
    return true;
  if (piece1.size() != piece2.size())
    return false;
  auto it2 = piece2.begin();
  for (auto ch1: piece1) {
    if (ch1 != *it2)
      return false;
    ++it2;
  }
  return true;
}

BASE_EXPORT bool operator==(const StringPiece16& piece1,
                            const StringPiece16& piece2) {
  if (piece1.data() == piece2.data())
    return true;
  if (piece1.size() != piece2.size())
    return false;
  auto it2 = piece2.begin();
  for (auto ch1: piece1) {
    if (ch1 != *it2)
      return false;
    ++it2;
  }
  return true;
}

BASE_EXPORT std::ostream& operator<<(std::ostream& ostream,
                                     const StringPiece& piece) {
  ostream.write(piece.data(), static_cast<std::streamsize>(piece.size()));
  return ostream;
}

}  // namespace base
