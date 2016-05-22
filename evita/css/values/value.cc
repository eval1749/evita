// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <ostream>

#include "evita/css/values/value.h"

#include "base/logging.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"
#include "evita/css/values.h"
#include "evita/css/values/color_value.h"
#include "evita/css/values/value_type.h"
#include "evita/gfx/base/colors/float_color.h"

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
Value::Value(const ColorValue& color) {
  data_.immediate.u64 = static_cast<uint64_t>(Tag::Immediate);
  data_.immediate.packed.type = ValueType::Color;
  data_.immediate.packed.data.u32 = color.value().ToRgba();
}

Value::Value(const Dimension& dimension) {
  data_.immediate.u64 = static_cast<uint64_t>(Tag::Immediate);
  data_.immediate.packed.type = ValueType::Dimension;
  data_.immediate.packed.data.f32 = dimension.number();
  data_.immediate.packed.unit = dimension.unit();
}

Value::Value(Keyword keyword) {
  data_.immediate.u64 = static_cast<uint64_t>(Tag::Immediate);
  data_.immediate.packed.type = ValueType::Keyword;
  data_.immediate.packed.data.u32 = static_cast<uint32_t>(keyword);
}

Value::Value(const Percentage& percentage) {
  data_.immediate.u64 = static_cast<uint64_t>(Tag::Immediate);
  data_.immediate.packed.type = ValueType::Percentage;
  data_.immediate.packed.data.f32 = percentage.value();
}

Value::Value(base::StringPiece16 string) {
  data_.string = new RefCountedString(string);
  data_.string->AddRef();
}

Value::Value(const String& string) {
  data_.string = string.value().get();
  if (!data_.string)
    return;
  data_.string->AddRef();
}

Value::Value(float value) {
  data_.immediate.u64 = static_cast<uint64_t>(Tag::Immediate);
  data_.immediate.packed.type = ValueType::Number;
  data_.immediate.packed.data.f32 = value;
}

Value::Value(int value) {
  data_.immediate.u64 = static_cast<uint64_t>(Tag::Immediate);
  data_.immediate.packed.type = ValueType::Integer;
  data_.immediate.packed.data.u32 = value;
}

Value::Value(const Value& other) {
  data_.immediate.u64 = other.data_.immediate.u64;
  if (!is_string())
    return;
  data_.string->AddRef();
}

Value::Value(Value&& other) {
  data_.immediate.u64 = other.data_.immediate.u64;
  other.DidMove();
}

Value::Value() {
  data_.immediate.u64 = static_cast<uint64_t>(Tag::Immediate);
  data_.immediate.packed.type = ValueType::Unspecified;
}

Value::~Value() {
  Reset();
}

Value& Value::operator=(const Value& other) {
  Reset();
  data_.immediate.u64 = other.data_.immediate.u64;
  if (!other.is_string())
    return *this;
  data_.string->AddRef();
  return *this;
}

Value& Value::operator=(Value&& other) {
  Reset();
  data_.immediate.u64 = other.data_.immediate.u64;
  other.DidMove();
  return *this;
}

bool Value::operator==(const Value& other) const {
  if (this == &other)
    return true;
  if (type() != other.type())
    return false;
  if (is_string())
    return other.is_string() && as_string() == other.as_string();
  return data_.immediate.u64 == other.data_.immediate.u64;
}

bool Value::operator!=(const Value& other) const {
  return !operator==(other);
}

ColorValue Value::as_color() const {
  DCHECK(is_color()) << *this;
  const auto rgba = data_.immediate.packed.data.u32;
  const auto red = static_cast<float>(rgba >> 24) / 255;
  const auto green = static_cast<float>((rgba >> 16) & 0xFF) / 255;
  const auto blue = static_cast<float>((rgba >> 8) & 0xFF) / 255;
  const auto alpha = static_cast<float>(rgba & 0xFF) / 255;
  return ColorValue(red, green, blue, alpha);
}

ColorValue Value::as_color_value() const {
  return as_color();
}

Dimension Value::as_dimension() const {
  DCHECK(is_dimension()) << *this;
  return Dimension(data_.immediate.packed.data.f32,
                   data_.immediate.packed.unit);
}

int Value::as_integer() const {
  DCHECK(is_integer()) << *this;
  return data_.immediate.packed.data.u32;
}

Keyword Value::as_keyword() const {
  DCHECK(is_keyword()) << *this;
  return static_cast<Keyword>(data_.immediate.packed.data.u32);
}

Length Value::as_length() const {
  DCHECK(is_length()) << *this;
  return Length(data_.immediate.packed.data.f32, data_.immediate.packed.unit);
}

float Value::as_number() const {
  DCHECK(is_number()) << *this;
  return data_.immediate.packed.data.f32;
}

Percentage Value::as_percentage() const {
  DCHECK(is_percentage()) << *this;
  return Percentage(data_.immediate.packed.data.f32);
}

String Value::as_string() const {
  DCHECK(is_string()) << *this;
  return String(data_.string);
}

bool Value::is_color() const {
  return type() == ValueType::Color;
}

bool Value::is_dimension() const {
  return type() == ValueType::Dimension;
}

bool Value::is_immediate() const {
  return data_.immediate.packed.tag == Tag::Immediate;
}

bool Value::is_integer() const {
  return type() == ValueType::Integer;
}

bool Value::is_keyword() const {
  return type() == ValueType::Keyword;
}

bool Value::is_length() const {
  return is_dimension() && Dimension::is_length(as_dimension().unit());
}

bool Value::is_number() const {
  return type() == ValueType::Number;
}

bool Value::is_percentage() const {
  return type() == ValueType::Percentage;
}

bool Value::is_string() const {
  return type() == ValueType::String;
}

bool Value::is_unspecified() const {
  return type() == ValueType::Unspecified;
}

const base::string16& Value::string_value() const {
  DCHECK(is_string()) << *this;
  return data_.string->data();
}

ValueType Value::type() const {
  if (is_immediate())
    return data_.immediate.packed.type;
  return ValueType::String;
}

#define V(Name, name)                                     \
  bool Value::is_##name() const {                         \
    return is_keyword() && as_keyword() == Keyword::Name; \
  }
FOR_EACH_VISUAL_CSS_KEYWORD_VALUE(V)
#undef V

void Value::DidMove() {
  data_.immediate.u64 = static_cast<uint64_t>(Tag::Immediate);
  data_.immediate.packed.type = ValueType::Unspecified;
}

void Value::Reset() {
  if (is_string())
    data_.string->Release();
  data_.immediate.u64 = static_cast<uint64_t>(Tag::Immediate);
  data_.immediate.packed.type = ValueType::Unspecified;
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
      return string_value();
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
