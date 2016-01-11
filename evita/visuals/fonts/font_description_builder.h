// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_FONTS_FONT_DESCRIPTION_BUILDER_H_
#define EVITA_VISUALS_FONTS_FONT_DESCRIPTION_BUILDER_H_

#include "evita/visuals/fonts/font_description.h"

#include "base/macros.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// FontDescription::Builder
//
class FontDescription::Builder final {
 public:
  Builder();
  ~Builder();

  FontDescription Build() { return description_; }
  Builder& SetFamily(const base::string16& family);
  Builder& SetSize(float size);
  Builder& SetStretch(FontStretch stretch);
  Builder& SetStyle(FontStyle style);
  Builder& SetWeight(FontWeight weight);

 private:
  FontDescription description_;

  DISALLOW_COPY_AND_ASSIGN(Builder);
};

}  // namespace visuals

#endif  // EVITA_VISUALS_FONTS_FONT_DESCRIPTION_BUILDER_H_
