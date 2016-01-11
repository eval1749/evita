// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/fonts/font_description.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// FontDescription
//
FontDescription::FontDescription(const FontDescription& other)
    : family_(other.family_),
      size_(other.size_),
      stretch_(other.stretch_),
      style_(other.style_),
      weight_(other.weight_) {}

FontDescription::FontDescription() {}
FontDescription::~FontDescription() {}

FontDescription& FontDescription::operator=(const FontDescription& other) {
  family_ = other.family_;
  size_ = other.size_;
  stretch_ = other.stretch_;
  style_ = other.style_;
  weight_ = other.weight_;
  return *this;
}

bool FontDescription::operator==(const FontDescription& other) const {
  return family_ == other.family_ && size_ == other.size_ &&
         stretch_ == other.stretch_ && style_ == other.style_ &&
         weight_ == other.weight_;
}

bool FontDescription::operator!=(const FontDescription& other) const {
  return !operator==(other);
}

}  // namespace visuals
