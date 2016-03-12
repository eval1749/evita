// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <ostream>

#include "evita/visuals/css/values/value.h"

#include "base/logging.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"
#include "evita/gfx/base/colors/float_color.h"
#include "evita/visuals/css/values.h"
#include "evita/visuals/css/values/color_value.h"
#include "evita/visuals/css/values/value_type.h"

namespace visuals {
namespace css {

namespace {

base::string16 FloatColorToString16(const gfx::FloatColor& color) {
  if (color.alpha() == 1)
    return base::StringPrintf(L"#%06X", color.ToRgba() >> 8);
  return base::StringPrintf(
      L"rgba(%d, %d, %d, %f)", static_cast<int>(color.red() * 255),
      static_cast<int>(color.green() * 255),
      static_cast<int>(color.blue() * 255), color.alpha());
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// Value
//
Value::Value(const ColorValue& color) : type_(ValueType::Color) {
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

Value::Value(base::StringPiece16 string) : type_(ValueType::String) {
  data_.string = new String(string);
}

Value::Value(const String& string) : type_(ValueType::String) {
  data_.string = new String(string);
}

Value::Value(float value) : type_(ValueType::Number) {
  data_.f32 = value;
}
Value::Value(int value) : type_(ValueType::Integer) {
  data_.u32 = value;
}

Value::Value(const Value& other) : data_(other.data_), type_(other.type_) {
  if (type_ != ValueType::String)
    return;
  data_.string = new String(*data_.string);
}

Value::Value(Value&& other) : data_(other.data_), type_(other.type_) {
  other.DidMove();
}

Value::Value() : type_(ValueType::Unspecified) {
  data_.u32 = 0;
}

Value::~Value() {
  Reset();
}

Value& Value::operator=(const Value& other) {
  Reset();
  type_ = other.type_;
  if (other.type_ == ValueType::String) {
    data_.string = new String(*other.data_.string);
    return *this;
  }
  data_ = other.data_;
  return *this;
}

Value& Value::operator=(Value&& other) {
  Reset();
  data_ = other.data_;
  type_ = other.type_;
  other.DidMove();
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
    case ValueType::String:
      return *data_.string == *other.data_.string;
  }
  NOTREACHED() << type_;
  return false;
}

bool Value::operator!=(const Value& other) const {
  return !operator==(other);
}

ColorValue Value::as_color() const {
  DCHECK(is_color()) << type();
  const auto red = static_cast<float>(data_.u32 >> 24) / 255;
  const auto green = static_cast<float>((data_.u32 >> 16) & 0xFF) / 255;
  const auto blue = static_cast<float>((data_.u32 >> 8) & 0xFF) / 255;
  const auto alpha = static_cast<float>(data_.u32 & 0xFF) / 255;
  return ColorValue(red, green, blue, alpha);
}

ColorValue Value::as_color_value() const {
  return as_color();
}

Dimension Value::as_dimension() const {
  DCHECK(is_dimension()) << type();
  return Dimension(data_.dimension.number, data_.dimension.unit);
}

int Value::as_integer() const {
  DCHECK(is_integer()) << type();
  return data_.u32;
}

Keyword Value::as_keyword() const {
  DCHECK(is_keyword()) << *this;
  return static_cast<Keyword>(data_.u32);
}

Length Value::as_length() const {
  DCHECK(is_length()) << type();
  return Length(data_.dimension.number, data_.dimension.unit);
}

float Value::as_number() const {
  DCHECK(is_number()) << type();
  return data_.f32;
}

Percentage Value::as_percentage() const {
  DCHECK(is_percentage()) << type();
  return Percentage(data_.f32);
}

const String& Value::as_string() const {
  DCHECK(is_string()) << *this;
  return *data_.string;
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

bool Value::is_length() const {
  return is_dimension() && Dimension::is_length(data_.dimension.unit);
}

bool Value::is_number() const {
  return type_ == ValueType::Number;
}

bool Value::is_percentage() const {
  return type_ == ValueType::Percentage;
}

bool Value::is_string() const {
  return type_ == ValueType::String;
}

bool Value::is_unspecified() const {
  return type_ == ValueType::Unspecified;
}

#define V(Name, name)                                                        \
  bool Value::is_##name() const {                                            \
    return is_keyword() && static_cast<Keyword>(data_.u32) == Keyword::Name; \
  }
FOR_EACH_VISUAL_CSS_KEYWORD_VALUE(V)
#undef V

void Value::DidMove() {
  data_.string = nullptr;
  type_ = ValueType::Unspecified;
}

void Value::Reset() {
  if (type_ == ValueType::String)
    delete data_.string;
  type_ = ValueType::Unspecified;
  data_.u32 = 0;
}

base::string16 Value::ToString16() const {
  switch (type()) {
    case ValueType::Color:
      return FloatColorToString16(as_color().value());
    case ValueType::Dimension: {
      const auto& dimension = as_dimension();
      return base::StringPrintf(L"%f%s", dimension.number(),
                                UnitToString(dimension.unit()));
    }
    case ValueType::Integer:
      return base::IntToString16(as_integer());
    case ValueType::Number:
      return base::UTF8ToUTF16(base::DoubleToString(as_number()));
    case ValueType::Keyword:
      return KeywordToString16(as_keyword());
    case ValueType::Percentage:
      return base::StringPrintf(L"%f%%", as_percentage().value());
    case ValueType::String:
      if (!as_string().value())
        return base::string16();
      return as_string().value()->data();
    case ValueType::Unspecified:
      return base::string16();
  }
  NOTREACHED() << *this;
  return base::string16();
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
    case ValueType::Keyword:
      return ostream << value.as_keyword();
    case ValueType::Percentage:
      return ostream << value.as_percentage();
    case ValueType::String:
      return ostream << value.as_string();
    case ValueType::Unspecified:
      return ostream << "Unspecified";
  }
  NOTREACHED() << value.type();
  return ostream << "???";
}

}  // namespace css
}  // namespace visuals
