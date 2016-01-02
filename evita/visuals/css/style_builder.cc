// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/css/style_builder.h"

#include "evita/visuals/css/style.h"
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

StyleBuilder& StyleBuilder::SetHeight(float height) {
  return SetHeight(css::Height(css::Length(height)));
}

StyleBuilder& StyleBuilder::SetWidth(float width) {
  return SetWidth(css::Width(css::Length(width)));
}

#define V(Name, name, type, text)                    \
  StyleBuilder& StyleBuilder::Set##Name(type name) { \
    style_->contains_.insert(PropertyId::Name);      \
    style_->name##_ = name;                          \
    return *this;                                    \
  }                                                  \
                                                     \
  void StyleBuilder::Unset##Name() {                 \
    style_->contains_.erase(PropertyId::Name);       \
  }

FOR_EACH_VISUAL_CSS_PROPERTY(V)
#undef V

}  // namespace css
}  // namespace visuals
