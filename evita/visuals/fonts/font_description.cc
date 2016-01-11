// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <ostream>

#include "base/strings/utf_string_conversions.h"
#include "evita/visuals/fonts/font_description.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// FontDescription
//
FontDescription::FontDescription(const Builder& builder,
                                 const FontDescription& other)
    : family_(other.family_),
      size_(other.size_),
      stretch_(other.stretch_),
      style_(other.style_),
      weight_(other.weight_) {}

FontDescription::FontDescription() {}
FontDescription::~FontDescription() {}

bool FontDescription::operator==(const FontDescription& other) const {
  if (this == &other)
    return true;
  return family_ == other.family_ && size_ == other.size_ &&
         stretch_ == other.stretch_ && style_ == other.style_ &&
         weight_ == other.weight_;
}

bool FontDescription::operator!=(const FontDescription& other) const {
  return !operator==(other);
}

std::ostream& operator<<(std::ostream& ostream,
                         const FontDescription& description) {
  return ostream << '"' << base::UTF16ToUTF8(description.family())
                 << "\" size=" << description.size()
                 << " stretch=" << static_cast<int>(description.stretch())
                 << " style=" << static_cast<int>(description.style())
                 << " weight=" << static_cast<int>(description.weight());
  return ostream;
}

}  // namespace visuals
