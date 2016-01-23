// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <ostream>
#include <string>

#include "evita/visuals/css/string.h"

#include "base/strings/utf_string_conversions.h"

namespace visuals {
namespace css {

String::String(base::StringPiece16 value) : value_(value.as_string()) {}
String::String(const String& other) : value_(other.value_) {}
String::String() {}
String::~String() {}

bool String::operator==(const String& other) const {
  return value_ == other.value_;
}

bool String::operator!=(const String& other) const {
  return !operator==(other);
}

std::ostream& operator<<(std::ostream& ostream, const String& string) {
  return ostream << '"' << base::UTF16ToUTF8(string.value()) << '"';
}

}  // namespace css
}  // namespace visuals
