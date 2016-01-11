// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_CSS_STRING_H_
#define EVITA_VISUALS_CSS_STRING_H_

#include <iosfwd>
#include <string>

#include "base/strings/string16.h"
#include "base/strings/string_piece.h"

namespace visuals {
namespace css {

//////////////////////////////////////////////////////////////////////
//
// String
//
class String final {
 public:
  explicit String(const base::StringPiece16& value);
  String(const String& other);
  String();
  ~String();

  bool operator==(const String& other) const;
  bool operator!=(const String& other) const;

  const base::string16& value() const { return value_; }

 private:
  base::string16 value_;
};

std::ostream& operator<<(std::ostream& ostream, const String& string);

}  // namespace css
}  // namespace visuals

#endif  // EVITA_VISUALS_CSS_STRING_H_
