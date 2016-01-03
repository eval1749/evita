// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_CSS_STYLE_EDITOR_H_
#define EVITA_VISUALS_CSS_STYLE_EDITOR_H_

#include <memory>

#include "base/macros.h"
#include "evita/visuals/css/properties_forward.h"
#include "evita/visuals/css/values_forward.h"

namespace visuals {
namespace css {

class Style;

//////////////////////////////////////////////////////////////////////
//
// StyleEditor
//
class StyleEditor final {
 public:
  StyleEditor();
  ~StyleEditor();

  // Merge |right| into |left| if |left| doesn't have property.
  void Merge(css::Style* left, const css::Style& right);

  // Shorthand functions
  void SetHeight(Style* style, float height);
  void SetWidth(Style* style, float width);

#define V(Name, name, type, text)          \
  void Set##Name(Style* style, type name); \
  void Unset##Name(Style* style);

  FOR_EACH_VISUAL_CSS_PROPERTY(V)
#undef V

 private:
  DISALLOW_COPY_AND_ASSIGN(StyleEditor);
};

}  // namespace css
}  // namespace visuals

#endif  // EVITA_VISUALS_CSS_STYLE_EDITOR_H_
