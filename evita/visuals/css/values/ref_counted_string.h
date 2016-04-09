// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_CSS_VALUES_REF_COUNTED_STRING_H_
#define EVITA_VISUALS_CSS_VALUES_REF_COUNTED_STRING_H_

#include <iosfwd>
#include <string>

#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "base/strings/string16.h"
#include "base/strings/string_piece.h"

namespace visuals {
namespace css {

//////////////////////////////////////////////////////////////////////
//
// RefCountedString
//
class RefCountedString final : public base::RefCounted<RefCountedString> {
 public:
  explicit RefCountedString(base::StringPiece16 data);
  RefCountedString();

  bool operator==(const RefCountedString& other) const;
  bool operator!=(const RefCountedString& other) const;

  const base::string16& data() const { return data_; }

 private:
  friend class base::RefCounted<RefCountedString>;

  ~RefCountedString();

  base::string16 data_;

  DISALLOW_COPY_AND_ASSIGN(RefCountedString);
};

std::ostream& operator<<(std::ostream& ostream, const RefCountedString* string);
std::ostream& operator<<(std::ostream& ostream, const RefCountedString& string);

}  // namespace css
}  // namespace visuals

#endif  // EVITA_VISUALS_CSS_VALUES_REF_COUNTED_STRING_H_
