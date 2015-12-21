// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMMON_STRINGS_ATOMIC_STRING_H_
#define COMMON_STRINGS_ATOMIC_STRING_H_

#include <iosfwd>
#include <string>

#include "base/strings/string16.h"
#include "common/common_export.h"

namespace common {

class COMMON_EXPORT AtomicString {
 public:
  explicit AtomicString(const base::string16& string);
  AtomicString(const AtomicString& other);
  AtomicString();
  ~AtomicString();

  operator const base::string16&() const { return *string_; }
  bool operator!() const { return empty(); }

  AtomicString& operator=(const AtomicString& other) {
    string_ = other.string_;
    return *this;
  }

  bool operator==(const AtomicString& other) const {
    return string_ == other.string_;
  }

  bool operator!=(const AtomicString& other) const {
    return !operator==(other);
  }

  bool empty() const { return string_->empty(); }
  const base::string16* get() const { return string_; }
  const base::string16& str() const { return *string_; }

  static const AtomicString& Empty();

 private:
  class Set;
  const base::string16* string_;
};

COMMON_EXPORT std::ostream& operator<<(std::ostream& ostream,
                                       const AtomicString& string);
COMMON_EXPORT std::ostream& operator<<(std::ostream& ostream,
                                       const AtomicString* string);

}  // namespace common

#endif  // COMMON_STRINGS_ATOMIC_STRING_H_
