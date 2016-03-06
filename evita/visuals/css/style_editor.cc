// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/css/style_editor.h"

#include "evita/visuals/css/float_color.h"
#include "evita/visuals/css/properties.h"
#include "evita/visuals/css/style.h"
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
    Set##Property(left, right.property##_);
  FOR_EACH_VISUAL_CSS_PROPERTY(V)
#undef V
}

void StyleEditor::SetBorder(Style* style,
                            const FloatColor& color,
                            float width) {
  SetBorderBottomWidth(style, Length(width));
  SetBorderLeftWidth(style, Length(width));
  SetBorderRightWidth(style, Length(width));
  SetBorderTopWidth(style, Length(width));

  SetBorderBottomColor(style, ColorValue(color));
  SetBorderLeftColor(style, ColorValue(color));
  SetBorderRightColor(style, ColorValue(color));
  SetBorderTopColor(style, ColorValue(color));
}

void StyleEditor::SetHeight(Style* style, float height) {
  SetHeight(style, Height(Length(height)));
}

void StyleEditor::SetPadding(Style* style, float width) {
  SetPaddingBottom(style, Length(width));
  SetPaddingLeft(style, Length(width));
  SetPaddingRight(style, Length(width));
  SetPaddingTop(style, Length(width));
}

void StyleEditor::SetWidth(Style* style, float width) {
  SetWidth(style, Width(Length(width)));
}

#define V(Name, name, type, text)                                  \
  void StyleEditor::Set##Name(Style* style, type name) {           \
    style->contains_.set(static_cast<size_t>(PropertyId::Name));   \
    style->name##_ = name;                                         \
  }                                                                \
                                                                   \
  void StyleEditor::Unset##Name(Style* style) {                    \
    style->contains_.reset(static_cast<size_t>(PropertyId::Name)); \
  }

FOR_EACH_VISUAL_CSS_PROPERTY(V)
#undef V

}  // namespace css
}  // namespace visuals
