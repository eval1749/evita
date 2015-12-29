// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_STYLE_STYLE_BUILDER_H_
#define EVITA_VISUALS_STYLE_STYLE_BUILDER_H_

#include <memory>

#include "base/macros.h"
#include "evita/visuals/style/style_properties_forward.h"

namespace visuals {
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

#define V(Name, name, type, text)     \
  StyleBuilder& Set##Name(type name); \
  void Unset##Name();

  FOR_EACH_VISUAL_STYLE_PROPERTY(V)
#undef V

 private:
  std::unique_ptr<Style> style_;

  DISALLOW_COPY_AND_ASSIGN(StyleBuilder);
};

}  // namespace css
}  // namespace visuals

#endif  // EVITA_VISUALS_STYLE_STYLE_BUILDER_H_
