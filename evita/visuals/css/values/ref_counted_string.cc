// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/css/values/ref_counted_string.h"

#include "base/strings/utf_string_conversions.h"

namespace visuals {
namespace css {

//////////////////////////////////////////////////////////////////////
//
// RefCountedString
//
RefCountedString::RefCountedString(base::StringPiece16 data)
    : data_(data.as_string()) {}

RefCountedString::RefCountedString() = default;
RefCountedString::~RefCountedString() = default;

bool RefCountedString::operator==(const RefCountedString& other) const {
  if (this == &other)
    return true;
  return data_ == other.data_;
}

bool RefCountedString::operator!=(const RefCountedString& other) const {
  return !operator==(other);
}

std::ostream& operator<<(std::ostream& ostream,
                         const RefCountedString* string) {
  if (!string)
    return ostream << "null";
  return ostream << *string;
}

std::ostream& operator<<(std::ostream& ostream,
                         const RefCountedString& string) {
  return ostream << '"' << base::UTF16ToUTF8(string.data()) << '"';
}

}  // namespace css
}  // namespace visuals
