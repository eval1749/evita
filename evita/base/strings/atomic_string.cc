// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <ostream>
#include <string>

#include "evita/base/strings/atomic_string.h"

#include "base/strings/utf_string_conversions.h"
#include "evita/base/strings/atomic_string_factory.h"

namespace evita {

//////////////////////////////////////////////////////////////////////
//
// AtomicString
//
AtomicString::AtomicString(const base::StringPiece16* value) : value_(value) {}

AtomicString::AtomicString(base::StringPiece16 value)
    : AtomicString(AtomicStringFactory::GetInstance()->New(value)) {}

AtomicString::AtomicString(const AtomicString& other) : value_(other.value_) {}

AtomicString::AtomicString() : AtomicString(L"") {}

AtomicString& AtomicString::operator=(const AtomicString& other) {
  value_ = other.value_;
  return *this;
}

AtomicString::~AtomicString() {}

bool AtomicString::operator==(const AtomicString& other) const {
  return value_ == other.value_;
}

bool AtomicString::operator==(base::StringPiece16 other) const {
  return *this == AtomicStringFactory::GetInstance()->New(other);
}

bool AtomicString::operator!=(const AtomicString& other) const {
  return !operator==(other);
}

bool AtomicString::operator!=(base::StringPiece16 other) const {
  return !operator==(other);
}

bool AtomicString::operator<(const AtomicString& other) const {
  return value_ < other.value_;
}

base::string16 AtomicString::as_string() const {
  return value_->as_string();
}

// static
AtomicString AtomicString::NewUniqueString(const base::char16* format) {
  return AtomicStringFactory::GetInstance()->NewUniqueString(format);
}

std::ostream& operator<<(std::ostream& ostream,
                         const AtomicString& atomic_string) {
  return ostream << base::UTF16ToUTF8(atomic_string.value().as_string());
}

std::ostream& operator<<(std::ostream& ostream,
                         const AtomicString* atomic_string) {
  if (!atomic_string)
    return ostream << "null";
  return ostream << *atomic_string;
}

}  // namespace evita
