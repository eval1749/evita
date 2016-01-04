// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/css/style_builder.h"

#include "evita/visuals/css/style.h"
#include "evita/visuals/css/style_editor.h"
#include "evita/visuals/css/properties.h"
#include "evita/visuals/css/values.h"

namespace visuals {
namespace css {

//////////////////////////////////////////////////////////////////////
//
// StyleBuilder
//
StyleBuilder::StyleBuilder(std::unique_ptr<Style> style)
    : style_(std::move(style)) {}
StyleBuilder::StyleBuilder() : style_(new Style()) {}
StyleBuilder::~StyleBuilder() {}

std::unique_ptr<Style> StyleBuilder::Build() {
  return std::move(style_);
}

StyleBuilder& StyleBuilder::SetColor(float red,
                                     float green,
                                     float blue,
                                     float alpha) {
  return SetColor(css::Color(red, green, blue, alpha));
}

StyleBuilder& StyleBuilder::SetHeight(float height) {
  return SetHeight(css::Height(css::Length(height)));
}

StyleBuilder& StyleBuilder::SetWidth(float width) {
  return SetWidth(css::Width(css::Length(width)));
}

#define V(Name, name, type, text)                    \
  StyleBuilder& StyleBuilder::Set##Name(type name) { \
    StyleEditor().Set##Name(style_.get(), name);     \
    return *this;                                    \
  }

FOR_EACH_VISUAL_CSS_PROPERTY(V)
#undef V

}  // namespace css
}  // namespace visuals
