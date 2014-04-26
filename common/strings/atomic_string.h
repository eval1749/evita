// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_common_strings_atomic_string_h)
#define INCLUDE_common_strings_atomic_string_h

#include "base/strings/string16.h"
#include "common/common_export.h"

namespace common {

class COMMON_EXPORT AtomicString {
  private: class Set;

  private: const base::string16* string_;

  public: explicit AtomicString(const base::string16& string);
  public: AtomicString(const AtomicString& other);
  public: AtomicString();
  public: ~AtomicString();

  public: operator const base::string16&() const {
    return *string_;
  }

  public: bool operator!() const { return empty(); }

  public: AtomicString& operator=(const AtomicString& other) {
    string_ = other.string_;
    return *this;
  }

  public: bool operator==(const AtomicString& other) const {
    return string_ == other.string_;
  }

  public: bool operator!=(const AtomicString& other) const {
    return !operator==(other);
  }

  public: bool empty() const { return string_->empty(); }
  public: const base::string16* get() const { return string_; }

  public: static const AtomicString& Empty();
};

}  // namespace common

#endif //!defined(INCLUDE_common_strings_atomic_string_h)
