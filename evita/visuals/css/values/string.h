// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_CSS_VALUES_STRING_H_
#define EVITA_VISUALS_CSS_VALUES_STRING_H_

#include <iosfwd>
#include <string>

#include "base/strings/string16.h"
#include "base/strings/string_piece.h"
#include "evita/visuals/css/values/ref_counted_string.h"

namespace visuals {
namespace css {

class RefCountedString;

//////////////////////////////////////////////////////////////////////
//
// String
//
class String final {
 public:
  explicit String(base::StringPiece16 value);
  explicit String(RefCountedString* value);
  String(const String& other);
  String(String&& other);
  String();
  ~String();

  String& operator=(const String& other);
  String& operator=(String&& other);

  bool operator==(const String& other) const;
  bool operator!=(const String& other) const;

  base::StringPiece16 data() const;
  scoped_refptr<RefCountedString> value() const { return value_; }

 private:
  scoped_refptr<RefCountedString> value_;
};

std::ostream& operator<<(std::ostream& ostream, const String& string);

}  // namespace css
}  // namespace visuals

#endif  // EVITA_VISUALS_CSS_VALUES_STRING_H_
