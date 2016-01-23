// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_BASE_STRINGS_ATOMIC_STRING_H_
#define EVITA_BASE_STRINGS_ATOMIC_STRING_H_

#include <functional>
#include <iosfwd>
#include <string>

#include "base/strings/string_piece.h"
#include "evita/base/evita_base_export.h"
#include "evita/base/memory/zone_allocated.h"

namespace evita {

//////////////////////////////////////////////////////////////////////
//
// AtomicString
//
class EVITA_BASE_EXPORT AtomicString final {
 public:
  explicit AtomicString(base::StringPiece16 value);
  AtomicString(const AtomicString& other);
  AtomicString();
  ~AtomicString();

  AtomicString& operator=(const AtomicString& other);

  bool operator==(const AtomicString& other) const;
  bool operator==(base::StringPiece16 other) const;
  bool operator!=(const AtomicString& other) const;
  bool operator!=(base::StringPiece16 other) const;
  bool operator<(const AtomicString& other) const;

  base::string16 as_string() const;
  bool empty() const { return value_->empty(); }
  base::StringPiece16 value() const { return *value_; }

  static AtomicString NewUniqueString(const base::char16* format);

 private:
  friend class AtomicStringFactory;

  explicit AtomicString(const base::StringPiece16* value);

  const base::StringPiece16* value_;
};

//////////////////////////////////////////////////////////////////////
//
// Printers
//
EVITA_BASE_EXPORT std::ostream& operator<<(std::ostream& ostream,
                                           const AtomicString& atomic_string);

EVITA_BASE_EXPORT std::ostream& operator<<(std::ostream& ostream,
                                           const AtomicString* atomic_string);

}  // namespace evita

namespace std {

//////////////////////////////////////////////////////////////////////
//
// std::hash<AtomicString>
//
template <>
struct std::hash<evita::AtomicString> {
  size_t operator()(const evita::AtomicString& atomic_string) const {
    return std::hash<base::StringPiece16>()(atomic_string.value());
  }
};

}  // namespace std

#endif  // EVITA_BASE_STRINGS_ATOMIC_STRING_H_
