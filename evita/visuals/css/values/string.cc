// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <ostream>
#include <string>

#include "evita/visuals/css/values/string.h"

#include "base/strings/utf_string_conversions.h"
#include "evita/visuals/css/values/ref_counted_string.h"

namespace visuals {
namespace css {

String::String(base::StringPiece16 value)
    : value_(new RefCountedString(value)) {}

String::String(RefCountedString* value) : value_(value) {}
String::String(const String& other) : value_(other.value_) {}
String::String(String&& other) : value_(std::move(other.value_)) {}
String::String() {}
String::~String() = default;

String& String::operator=(const String& other) {
  value_ = other.value_;
  return *this;
}

String& String::operator=(String&& other) {
  value_ = std::move(other.value_);
  return *this;
}

bool String::operator==(const String& other) const {
  if (!value_)
    return !other.value_;
  return *value_ == *other.value_;
}

bool String::operator!=(const String& other) const {
  return !operator==(other);
}

base::StringPiece16 String::data() const {
  if (!value_)
    return base::StringPiece16();
  return base::StringPiece16(value_->data());
}

std::ostream& operator<<(std::ostream& ostream, const String& string) {
  return ostream << string.value().get();
}

}  // namespace css
}  // namespace visuals
