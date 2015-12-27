// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/style/style_builder.h"

#include "evita/visuals/style/style.h"
#include "evita/visuals/style/style_properties.h"

namespace visuals {

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

#define V(Name, name, type, text)                    \
  StyleBuilder& StyleBuilder::Set##Name(type name) { \
    style_->contains_.insert(StylePropertyId::Name); \
    style_->name##_ = name;                          \
    return *this;                                    \
  }                                                  \
                                                     \
  void StyleBuilder::Unset##Name() {                 \
    style_->contains_.erase(StylePropertyId::Name);  \
  }

FOR_EACH_VISUAL_STYLE_PROPERTY(V)
#undef V

}  // namespace visuals
