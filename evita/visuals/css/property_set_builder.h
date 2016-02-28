// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_CSS_PROPERTY_SET_BUILDER_H_
#define EVITA_VISUALS_CSS_PROPERTY_SET_BUILDER_H_

#include <memory>

#include "base/macros.h"
#include "evita/visuals/css/property_set.h"

namespace visuals {
namespace css {

//////////////////////////////////////////////////////////////////////
//
// PropertySet::Builder
//
class PropertySet::Builder final {
 public:
  Builder();
  ~Builder();

  Builder& AddColor(PropertyId property_id, const Color& color);
  Builder& AddDimension(PropertyId property_id, const Dimension& dimension);
  Builder& AddInteger(PropertyId property_id, int value);
  Builder& AddKeyword(PropertyId property_id, Keyword keyword);
  Builder& AddNumber(PropertyId property_id, float value);
  Builder& AddPercentage(PropertyId property_id, const Percentage& percentage);
  Builder& AddPercentage(PropertyId property_id, float value);
  PropertySet Build();

 private:
  void Add(PropertyId property_id, ValueType type, uint32_t data);
  void AddFloat(float data);
  void AddUint32(uint32_t data);
  static uint32_t EncodeSmallDimension(const Dimension& dimension);
  static uint32_t EncodeSmallInteger(int value);
  static uint32_t EncodeSmallNumber(float value);
  static bool IsSmallDimension(float value);
  static bool IsSmallInteger(int value);
  static bool IsSmallNumber(float value);

  std::unique_ptr<PropertySet> list_;

  DISALLOW_COPY_AND_ASSIGN(Builder);
};

}  // namespace css
}  // namespace visuals

#endif  // EVITA_VISUALS_CSS_PROPERTY_SET_BUILDER_H_
