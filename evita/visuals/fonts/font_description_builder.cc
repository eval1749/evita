// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/fonts/font_description_builder.h"

namespace visuals {

using Builder = FontDescription::Builder;

//////////////////////////////////////////////////////////////////////
//
// FontDescription::Builder
//
Builder::Builder() {}
Builder::~Builder() {}

Builder& Builder::SetFamily(const base::string16& family) {
  description_.family_ = family;
  return *this;
}

Builder& Builder::SetSize(float size) {
  description_.size_ = size;
  return *this;
}

Builder& Builder::SetStretch(FontStretch stretch) {
  description_.stretch_ = stretch;
  return *this;
}

Builder& Builder::SetStyle(FontStyle style) {
  description_.style_ = style;
  return *this;
}

Builder& Builder::SetWeight(FontWeight weight) {
  description_.weight_ = weight;
  return *this;
}

}  // namespace visuals
