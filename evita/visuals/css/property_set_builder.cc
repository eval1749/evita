// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <cmath>

#include "evita/visuals/css/property_set_builder.h"

#include "base/logging.h"
#include "evita/visuals/css/color.h"
#include "evita/visuals/css/dimension.h"
#include "evita/visuals/css/percentage.h"
#include "evita/visuals/css/property_set.h"
#include "evita/visuals/css/value_type.h"

namespace visuals {
namespace css {

const auto kUnitBits = 5;

using Builder = PropertySet::Builder;

//////////////////////////////////////////////////////////////////////
//
// Builder
//
Builder::Builder() : list_(new PropertySet()) {}
Builder::~Builder() {}

void Builder::Add(PropertyId property_id, ValueType type, uint32_t data) {
  DCHECK_LE(data, 1u << (kDataBits - 1));
  Word word;
  word.bits.property_id = property_id;
  word.bits.type = type;
  word.bits.data = data;
  list_->words_.emplace_back(word);
}

Builder& Builder::AddColor(PropertyId property_id, const Color& color) {
  Add(property_id, ValueType::Color, 0);
  AddUint32(color.value().ToRgba());
  return *this;
}

Builder& Builder::AddDimension(PropertyId property_id,
                               const Dimension& dimension) {
  if (IsSmallDimension(dimension.number())) {
    Add(property_id, ValueType::Dimension, EncodeSmallDimension(dimension));
    return *this;
  }
  Add(property_id, ValueType::Dimension, static_cast<int>(dimension.unit())
                                             << 1);
  AddFloat(dimension.number());
  return *this;
}

void Builder::AddFloat(float data) {
  Word word;
  word.f32 = data;
  list_->words_.emplace_back(word);
}

Builder& Builder::AddInteger(PropertyId property_id, int value) {
  if (IsSmallInteger(value)) {
    Add(property_id, ValueType::Integer, EncodeSmallInteger(value));
    return *this;
  }
  Add(property_id, ValueType::Integer, 0);
  AddUint32(value);
  return *this;
}

Builder& Builder::AddKeyword(PropertyId property_id, Keyword keyword) {
  Add(property_id, ValueType::Keyword, static_cast<uint32_t>(keyword));
  return *this;
}

Builder& Builder::AddNumber(PropertyId property_id, float value) {
  if (IsSmallNumber(value)) {
    Add(property_id, ValueType::Number, EncodeSmallNumber(value));
    return *this;
  }
  Add(property_id, ValueType::Number, 0);
  AddFloat(value);
  return *this;
}

Builder& Builder::AddPercentage(PropertyId property_id,
                                const Percentage& percentage) {
  return AddPercentage(property_id, percentage.value());
}

Builder& Builder::AddPercentage(PropertyId property_id, float value) {
  if (IsSmallNumber(value)) {
    Add(property_id, ValueType::Percentage, EncodeSmallNumber(value));
    return *this;
  }
  Add(property_id, ValueType::Percentage, 0);
  AddFloat(value);
  return *this;
}

void Builder::AddUint32(uint32_t data) {
  Word word;
  word.u32 = data;
  list_->words_.emplace_back(word);
}

PropertySet Builder::Build() {
  return *list_;
}

uint32_t Builder::EncodeSmallDimension(const Dimension& dimension) {
  DCHECK(IsSmallDimension(dimension.number())) << dimension;
  return (static_cast<uint32_t>(dimension.number() * 1000) << (kUnitBits + 1)) |
         (static_cast<int>(dimension.unit()) << 1) | 1;
}

uint32_t Builder::EncodeSmallInteger(int value) {
  DCHECK(IsSmallInteger(value)) << value;
  return (value << 1) | 1;
}

uint32_t Builder::EncodeSmallNumber(float value) {
  DCHECK(IsSmallNumber(value)) << value;
  return Builder::EncodeSmallInteger(static_cast<int>(value * 1000));
}

bool Builder::IsSmallDimension(float number) {
  // 1=sign, 1=is_small
  const auto kSmallValueBits = kDataBits - kUnitBits - 2;
  const auto scaled = number * 1000;
  if (std::trunc(scaled) != scaled)
    return false;
  if (scaled < static_cast<float>(-1 << kSmallValueBits))
    return false;
  if (scaled > static_cast<float>(1 << kSmallValueBits))
    return false;
  return true;
}

bool Builder::IsSmallInteger(int value) {
  // 1=sign, 1=is_small
  const auto kSmallValueBits = kDataBits - 2;
  return value >= (-1 << kSmallValueBits) && value < (1 << kSmallValueBits);
}

bool Builder::IsSmallNumber(float value) {
  const auto kSmallValueBits = kDataBits - 2;
  // 1=sign, 1=is_small
  const auto scaled = value * 1000;
  if (std::trunc(scaled) != scaled)
    return false;
  if (scaled < static_cast<float>(-1 << kSmallValueBits))
    return false;
  if (scaled > static_cast<float>(1 << kSmallValueBits))
    return false;
  return true;
}

}  // namespace css
}  // namespace visuals
