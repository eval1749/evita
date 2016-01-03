// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/css/style_editor.h"

#include "evita/visuals/css/style.h"
#include "evita/visuals/css/properties.h"
#include "evita/visuals/css/values.h"

namespace visuals {
namespace css {

//////////////////////////////////////////////////////////////////////
//
// StyleEditor
//
StyleEditor::StyleEditor() {}
StyleEditor::~StyleEditor() {}

void StyleEditor::Merge(Style* left, const Style& right) {
#define V(Property, property, ...)                       \
  if (!left->has_##property() && right.has_##property()) \
    left->property##_ = right.property##_;
  FOR_EACH_VISUAL_CSS_PROPERTY(V)
#undef V
}

void StyleEditor::SetHeight(Style* style, float height) {
  SetHeight(style, css::Height(css::Length(height)));
}

void StyleEditor::SetWidth(Style* style, float width) {
  SetWidth(style, css::Width(css::Length(width)));
}

#define V(Name, name, type, text)                        \
  void StyleEditor::Set##Name(Style* style, type name) { \
    style->contains_.insert(PropertyId::Name);           \
    style->name##_ = name;                               \
  }                                                      \
                                                         \
  void StyleEditor::Unset##Name(Style* style) {          \
    style->contains_.erase(PropertyId::Name);            \
  }

FOR_EACH_VISUAL_CSS_PROPERTY(V)
#undef V

}  // namespace css
}  // namespace visuals
