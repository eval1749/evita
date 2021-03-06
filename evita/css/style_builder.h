// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_CSS_STYLE_BUILDER_H_
#define EVITA_CSS_STYLE_BUILDER_H_

#include <memory>

#include "base/macros.h"
#include "evita/css/properties_forward.h"
#include "evita/css/values_forward.h"

namespace css {

class Style;

//////////////////////////////////////////////////////////////////////
//
// StyleBuilder
//
class StyleBuilder final {
 public:
  explicit StyleBuilder(std::unique_ptr<Style> style);
  StyleBuilder();
  ~StyleBuilder();

  std::unique_ptr<Style> Build();

  StyleBuilder& Set(PropertyId id, const ColorValue& color);
  StyleBuilder& Set(PropertyId id, const Length& length);
  StyleBuilder& Set(PropertyId id, const Value& value);

  // Shorthand functions
  StyleBuilder& SetBorder(const ColorValue& color, float width);
  StyleBuilder& SetColor(float red, float green, float blue, float alpha = 1);
  StyleBuilder& SetPadding(float height);

#define V(Name, name, type, text) \
  StyleBuilder& Set##Name(const ColorValue& color);
  FOR_EACH_VISUAL_CSS_COLOR_PROPERTY(V)
#undef V

#define V(Name, name, type, text) StyleBuilder& Set##Name(float length);
  FOR_EACH_VISUAL_CSS_LENGTH_PROPERTY(V)
#undef V

// Full functions
#define V(Name, name, type, text) StyleBuilder& Set##Name(type name);
  FOR_EACH_VISUAL_CSS_PROPERTY(V)
#undef V

 private:
  std::unique_ptr<Style> style_;

  DISALLOW_COPY_AND_ASSIGN(StyleBuilder);
};

}  // namespace css

#endif  // EVITA_CSS_STYLE_BUILDER_H_
