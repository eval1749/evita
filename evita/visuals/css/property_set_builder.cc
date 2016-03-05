// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/css/property_set_builder.h"

#include "evita/visuals/css/percentage.h"
#include "evita/visuals/css/property_set.h"
#include "evita/visuals/css/property_set_editor.h"
#include "evita/visuals/css/value.h"
#include "evita/visuals/css/value_type.h"

namespace visuals {
namespace css {

using Builder = PropertySet::Builder;
using Editor = PropertySet::Editor;

//////////////////////////////////////////////////////////////////////
//
// Builder
//
Builder::Builder() : property_set_(new PropertySet()) {}
Builder::~Builder() {}

Builder& Builder::AddColor(PropertyId property_id, const Color& color) {
  Editor().Add(property_set_.get(), property_id, Value(color));
  return *this;
}

Builder& Builder::AddDimension(PropertyId property_id,
                               const Dimension& dimension) {
  Editor().Add(property_set_.get(), property_id, Value(dimension));
  return *this;
}

Builder& Builder::AddInteger(PropertyId property_id, int value) {
  Editor().Add(property_set_.get(), property_id, Value(value));
  return *this;
}

Builder& Builder::AddKeyword(PropertyId property_id, Keyword keyword) {
  Editor().Add(property_set_.get(), property_id, Value(keyword));
  return *this;
}

Builder& Builder::AddNumber(PropertyId property_id, float value) {
  Editor().Add(property_set_.get(), property_id, Value(value));
  return *this;
}

Builder& Builder::AddPercentage(PropertyId property_id,
                                const Percentage& value) {
  Editor().Add(property_set_.get(), property_id, Value(value));
  return *this;
}

Builder& Builder::AddPercentage(PropertyId property_id, float value) {
  Editor().Add(property_set_.get(), property_id, Value(Percentage(value)));
  return *this;
}

PropertySet Builder::Build() {
  return *property_set_;
}

}  // namespace css
}  // namespace visuals
