// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <ostream>

#include "evita/visuals/css/value.h"

#include "base/logging.h"
#include "evita/visuals/css/color.h"
#include "evita/visuals/css/dimension.h"
#include "evita/visuals/css/percentage.h"
#include "evita/visuals/css/unit.h"
#include "evita/visuals/css/value_type.h"

namespace visuals {
namespace css {

//////////////////////////////////////////////////////////////////////
//
// Value
//
Value::Value(const Color& color) : type_(ValueType::Color) {
  data_.u32 = color.value().ToRgba();
}

Value::Value(const Dimension& dimension) : type_(ValueType::Dimension) {
  data_.dimension.number = dimension.number();
  data_.dimension.unit = dimension.unit();
}

Value::Value(Keyword keyword) : type_(ValueType::Keyword) {
  data_.u32 = static_cast<uint32_t>(keyword);
}

Value::Value(const Percentage& percentage) : type_(ValueType::Percentage) {
  data_.f32 = percentage.value();
}
Value::Value(float value) : type_(ValueType::Number) {
  data_.f32 = value;
}
Value::Value(int value) : type_(ValueType::Integer) {
  data_.u32 = value;
}
Value::Value(const Value& other) : data_(other.data_), type_(other.type_) {}

Value::Value(Value&& other) : data_(other.data_), type_(other.type_) {
  other.data_.u32 = 0;
  other.type_ = ValueType::None;
}

Value::Value() : type_(ValueType::None) {
  data_.u32 = 0;
}

Value::~Value() {}

Value& Value::operator=(const Value& other) {
  data_ = other.data_;
  type_ = other.type_;
  return *this;
}

Value& Value::operator=(Value&& other) {
  data_ = other.data_;
  type_ = other.type_;
  other.data_.u32 = 0;
  other.type_ = ValueType::None;
  return *this;
}

bool Value::operator==(const Value& other) const {
  if (this == &other)
    return true;
  if (type_ != other.type_)
    return false;
  switch (type_) {
    case ValueType::Color:
    case ValueType::Integer:
    case ValueType::Keyword:
      return data_.u32 == other.data_.u32;
    case ValueType::Dimension:
      return data_.dimension.number == other.data_.dimension.number &&
             data_.dimension.unit == other.data_.dimension.unit;
    case ValueType::Number:
    case ValueType::Percentage:
      return data_.f32 == other.data_.f32;
  }
  NOTREACHED() << type_;
  return false;
}

bool Value::operator!=(const Value& other) const {
  return !operator==(other);
}

Color Value::as_color() const {
  DCHECK(is_color()) << type();
  const auto red = static_cast<float>(data_.u32 >> 24) / 255;
  const auto green = static_cast<float>((data_.u32 >> 16) & 0xFF) / 255;
  const auto blue = static_cast<float>((data_.u32 >> 8) & 0xFF) / 255;
  const auto alpha = static_cast<float>(data_.u32 & 0xFF) / 255;
  return Color(red, green, blue, alpha);
}

Dimension Value::as_dimension() const {
  DCHECK(is_dimension()) << type();
  return Dimension(data_.dimension.number, data_.dimension.unit);
}

int Value::as_integer() const {
  DCHECK(is_integer()) << type();
  return data_.u32;
}

float Value::as_number() const {
  DCHECK(is_number()) << type();
  return data_.f32;
}

Percentage Value::as_percentage() const {
  DCHECK(is_percentage()) << type();
  return Percentage(data_.f32);
}

bool Value::is_color() const {
  return type_ == ValueType::Color;
}
bool Value::is_dimension() const {
  return type_ == ValueType::Dimension;
}
bool Value::is_integer() const {
  return type_ == ValueType::Integer;
}
bool Value::is_keyword() const {
  return type_ == ValueType::Keyword;
}
bool Value::is_number() const {
  return type_ == ValueType::Number;
}
bool Value::is_percentage() const {
  return type_ == ValueType::Percentage;
}

std::ostream& operator<<(std::ostream& ostream, const Value& value) {
  switch (value.type()) {
    case ValueType::Color:
      return ostream << value.as_color();
    case ValueType::Dimension:
      return ostream << value.as_dimension();
    case ValueType::Integer:
      return ostream << value.as_integer();
    case ValueType::Number:
      return ostream << value.as_number();
    case ValueType::None:
      return ostream << "None";
    case ValueType::Percentage:
      return ostream << value.as_percentage();
  }
  NOTREACHED() << value.type();
  return ostream << "???";
}

}  // namespace css
}  // namespace visuals
