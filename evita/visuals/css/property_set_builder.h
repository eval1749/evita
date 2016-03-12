// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_CSS_PROPERTY_SET_BUILDER_H_
#define EVITA_VISUALS_CSS_PROPERTY_SET_BUILDER_H_

#include <memory>
#include <string>

#include "base/macros.h"
#include "base/strings/string_piece.h"
#include "evita/visuals/css/property_set.h"

namespace visuals {
namespace css {

enum class Unit : uint32_t;

//////////////////////////////////////////////////////////////////////
//
// PropertySet::Builder
//
class PropertySet::Builder final {
 public:
  Builder();
  ~Builder();

  Builder& AddColor(PropertyId property_id, const ColorValue& color);
  Builder& AddDimension(PropertyId property_id, const Dimension& dimension);
  Builder& AddInteger(PropertyId property_id, int value);
  Builder& AddKeyword(PropertyId property_id, Keyword keyword);
  Builder& AddNumber(PropertyId property_id, float value);
  Builder& AddPercentage(PropertyId property_id, const Percentage& percentage);
  Builder& AddPercentage(PropertyId property_id, float value);
  Builder& AddString(PropertyId property_id, base::StringPiece16 string);
  PropertySet Build();

 private:
  std::unique_ptr<PropertySet> property_set_;

  DISALLOW_COPY_AND_ASSIGN(Builder);
};

}  // namespace css
}  // namespace visuals

#endif  // EVITA_VISUALS_CSS_PROPERTY_SET_BUILDER_H_
