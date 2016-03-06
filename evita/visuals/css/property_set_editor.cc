// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>
#include <cmath>
#include <unordered_set>

#include "evita/visuals/css/property_set_editor.h"

#include "base/logging.h"
#include "evita/visuals/css/properties.h"
#include "evita/visuals/css/property_set.h"
#include "evita/visuals/css/values/color_value.h"
#include "evita/visuals/css/values/dimension.h"
#include "evita/visuals/css/values/percentage.h"
#include "evita/visuals/css/values/string.h"
#include "evita/visuals/css/values/value.h"
#include "evita/visuals/css/values/value_type.h"

namespace visuals {
namespace css {

const auto kUnitBits = 5;

using Editor = PropertySet::Editor;

//////////////////////////////////////////////////////////////////////
//
// Editor
//
Editor::Editor() {}
Editor::~Editor() {}

void Editor::Add(PropertySet* property_set,
                 PropertyId property_id,
                 const Value& value) {
  DCHECK(!property_set->Contains(property_id))
      << "Property '" << property_id << "' is already in " << *property_set;
  const auto value_size = SizeOfEncodedValue(value);
  const auto size = property_set->words_.size();
  property_set->words_.resize(size + value_size);
  EncodeValue(property_set->words_.begin() + size, property_id, value);
}

void Editor::Merge(PropertySet* left, const PropertySet& right) {
  std::unordered_set<PropertyId> left_set;
  for (const auto& left_property : *left)
    left_set.insert(left_property.id());
  for (const auto& right_property : right) {
    if (left_set.count(right_property.id()))
      continue;
    Add(left, right_property.id(), right_property.value());
  }
}

void Editor::EncodeValue(const std::vector<PropertySet::Word>::iterator& it,
                         PropertyId property_id,
                         const Value& value) {
  it->bits.property_id = property_id;
  it->bits.type = value.type();
  switch (value.type()) {
    case ValueType::Color:
      it->bits.data = 0;
      std::next(it)->u32 = value.as_color().value().ToRgba();
      return;
    case ValueType::Dimension: {
      const auto& dimension = value.as_dimension();
      if (IsSmallDimension(dimension)) {
        it->bits.data = EncodeSmallDimension(dimension);
        return;
      }
      it->bits.data = EncodeUnit(dimension.unit());
      std::next(it)->f32 = dimension.number();
      return;
    }
    case ValueType::Integer:
      if (IsSmallInteger(value.as_integer())) {
        it->bits.data = EncodeSmallInteger(value.as_integer());
        return;
      }
      it->bits.data = 0;
      std::next(it)->u32 = value.as_integer();
      return;
    case ValueType::Keyword:
      it->bits.data = static_cast<uint32_t>(value.as_keyword());
      return;
    case ValueType::Number:
      if (IsSmallNumber(value.as_number())) {
        it->bits.data = EncodeSmallNumber(value.as_number());
        return;
      }
      it->bits.data = 0;
      std::next(it)->f32 = value.as_number();
      return;
    case ValueType::Percentage: {
      const auto number = value.as_percentage().value();
      if (IsSmallNumber(number)) {
        it->bits.data = EncodeSmallNumber(number);
        return;
      }
      it->bits.data = 0;
      std::next(it)->f32 = number;
      return;
    }
    case ValueType::String: {
      const auto string_size = value.as_string().value().size();
      it->bits.data = static_cast<uint32_t>(string_size);
      ::memcpy(&it[1], value.as_string().value().data(),
               sizeof(base::char16) * string_size);
      return;
    }
  }
  NOTREACHED() << value;
}

void Editor::Remove(PropertySet* property_set, PropertyId property_id) {
  auto& words = property_set->words_;
  auto it = words.begin();
  const auto& end = words.end();
  while (it < end) {
    const auto value_size =
        PropertySet::SizeOfEncodedValue(it->bits.type, it->bits.data);
    const auto& next = it + value_size;
    if (it->bits.property_id != property_id) {
      it = next;
      continue;
    }
    std::move(next, end, it);
    words.resize(words.size() - value_size);
    return;
  }
  NOTREACHED() << "Property '" << property_id << "' isn't in " << *property_set;
}

void Editor::Set(PropertySet* property_set,
                 PropertyId property_id,
                 const Value& new_value) {
  auto& words = property_set->words_;
  auto it = words.begin();
  const auto& end = words.end();
  while (it < end) {
    const auto value_size =
        PropertySet::SizeOfEncodedValue(it->bits.type, it->bits.data);
    const auto& next = it + value_size;
    if (it->bits.property_id != property_id) {
      it = next;
      continue;
    }
    const auto new_value_size = SizeOfEncodedValue(new_value);
    const auto delta = new_value_size - value_size;
    if (delta < 0) {
      // Shrink word list and present slot for |new_value|
      std::move(next, end, it + new_value_size);
      words.resize(words.size() + delta);
    } else if (delta > 0) {
      // Enlarge word list and make room for |new_value|.
      words.resize(words.size() + delta);
      std::move_backward(next, end, it + new_value_size);
    }
    return EncodeValue(it, property_id, new_value);
  }
  Add(property_set, property_id, new_value);
}

#define V(Name, name, type, text)                                         \
  void Editor::Set##Name(PropertySet* property_set, const Value& value) { \
    Set(property_set, PropertyId::Name, value);                           \
  }                                                                       \
  void Editor::Remove##Name(PropertySet* property_set) {                  \
    Remove(property_set, PropertyId::Name);                               \
  }

FOR_EACH_VISUAL_CSS_PROPERTY(V)
#undef V

// static
uint32_t Editor::EncodeSmallDimension(const Dimension& dimension) {
  DCHECK(IsSmallDimension(dimension)) << dimension;
  return (static_cast<uint32_t>(dimension.number() * 1000) << (kUnitBits + 1)) |
         EncodeUnit(dimension.unit()) | 1;
}

// static
uint32_t Editor::EncodeSmallInteger(int value) {
  DCHECK(IsSmallInteger(value)) << value;
  return (value << 1) | 1;
}

// static
uint32_t Editor::EncodeSmallNumber(float value) {
  DCHECK(IsSmallNumber(value)) << value;
  return Editor::EncodeSmallInteger(static_cast<int>(value * 1000));
}

// static
uint32_t Editor::EncodeUnit(Unit unit) {
  return static_cast<uint32_t>(unit) << 1;
}

// static
bool Editor::IsSmallDimension(const Dimension& dimension) {
  const auto number = dimension.number();
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

// static
bool Editor::IsSmallInteger(int value) {
  // 1=sign, 1=is_small
  const auto kSmallValueBits = kDataBits - 2;
  return value >= (-1 << kSmallValueBits) && value < (1 << kSmallValueBits);
}

// static
bool Editor::IsSmallNumber(float value) {
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

bool Editor::IsSmallPercentage(const Percentage& percentage) {
  return IsSmallNumber(percentage.value());
}

size_t Editor::SizeOfEncodedValue(const Value& value) {
  switch (value.type()) {
    case ValueType::Color:
      return 2;
    case ValueType::Dimension:
      return IsSmallDimension(value.as_dimension()) ? 1 : 2;
    case ValueType::Integer:
      return IsSmallInteger(value.as_integer()) ? 1 : 2;
    case ValueType::Number:
      return IsSmallNumber(value.as_number()) ? 1 : 2;
    case ValueType::Percentage:
      return IsSmallPercentage(value.as_percentage()) ? 1 : 2;
    case ValueType::Keyword:
      return 1;
    case ValueType::String:
      return (value.as_string().value().size() + 1) / 2 + 1;
  }
  NOTREACHED() << value.type();
  return 0;
}

}  // namespace css
}  // namespace visuals
