// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/css/style_editor.h"

#include "evita/css/properties.h"
#include "evita/css/property_set_editor.h"
#include "evita/css/style.h"
#include "evita/css/values.h"
#include "evita/gfx/base/colors/float_color.h"

namespace css {

//////////////////////////////////////////////////////////////////////
//
// StyleEditor
//
StyleEditor::StyleEditor() {}
StyleEditor::~StyleEditor() {}

void StyleEditor::Merge(Style* left, const Style& right) {
  PropertySet::Editor().Merge(&left->properties_, right.properties());
}

void StyleEditor::Set(Style* style, PropertyId id, const ColorValue& color) {
  Set(style, id, Value(color));
}

void StyleEditor::Set(Style* style, PropertyId id, const Length& length) {
  Set(style, id, Value(length));
}

void StyleEditor::Set(Style* style, PropertyId id, const Value& value) {
  PropertySet::Editor().Set(&style->properties_, id, value);
}

void StyleEditor::SetBorder(Style* style,
                            const gfx::FloatColor& color,
                            float width) {
  SetBorderBottomStyle(style, BorderBottomStyle::Solid());
  SetBorderLeftStyle(style, BorderLeftStyle::Solid());
  SetBorderRightStyle(style, BorderRightStyle::Solid());
  SetBorderTopStyle(style, BorderTopStyle::Solid());

  SetBorderBottomWidth(style, width);
  SetBorderLeftWidth(style, width);
  SetBorderRightWidth(style, width);
  SetBorderTopWidth(style, width);

  SetBorderBottomColor(style, ColorValue(color));
  SetBorderLeftColor(style, ColorValue(color));
  SetBorderRightColor(style, ColorValue(color));
  SetBorderTopColor(style, ColorValue(color));
}

void StyleEditor::SetPadding(Style* style, float width) {
  SetPaddingBottom(style, width);
  SetPaddingLeft(style, width);
  SetPaddingRight(style, width);
  SetPaddingTop(style, width);
}

#define V(Name, name, type, text)                                      \
  void StyleEditor::Set##Name(Style* style, const ColorValue& color) { \
    return Set(style, PropertyId::Name, color);                        \
  }
FOR_EACH_VISUAL_CSS_COLOR_PROPERTY(V)
#undef V

#define V(Name, name, type, text)                           \
  void StyleEditor::Set##Name(Style* style, float length) { \
    return Set(style, PropertyId::Name, Length(length));    \
  }
FOR_EACH_VISUAL_CSS_LENGTH_PROPERTY(V)
#undef V

#define V(Name, name, type, text)                                        \
  void StyleEditor::Set##Name(Style* style, type name) {                 \
    PropertySet::Editor().Set(&style->properties_, PropertyId::Name,     \
                              name.value());                             \
  }                                                                      \
                                                                         \
  void StyleEditor::Unset##Name(Style* style) {                          \
    PropertySet::Editor().Remove(&style->properties_, PropertyId::Name); \
  }

FOR_EACH_VISUAL_CSS_PROPERTY(V)
#undef V

}  // namespace css
