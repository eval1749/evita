// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_CSS_VALUE_H_
#define EVITA_VISUALS_CSS_VALUE_H_

#include <stdint.h>

#include <iosfwd>

#include "evita/visuals/css/values_forward.h"

namespace visuals {
namespace css {

class Color;
class Dimension;
enum class Keyword;
class Percentage;
enum class Unit;
enum class ValueType : uint32_t;

//////////////////////////////////////////////////////////////////////
//
// Value
//
class Value final {
 public:
  explicit Value(const Color& color);
  explicit Value(const Dimension& dimension);
  explicit Value(Keyword keyword);
  explicit Value(const Percentage& percentage);
  explicit Value(float value);
  explicit Value(int value);
  Value(const Value& other);
  Value(Value&& other);
  Value();
  ~Value();

  Value& operator=(const Value& other);
  Value& operator=(Value&& other);

  bool operator==(const Value& other) const;
  bool operator!=(const Value& other) const;

  Color as_color() const;
  Dimension as_dimension() const;
  int as_integer() const;
  Keyword as_keyword() const;
  float as_number() const;
  Percentage as_percentage() const;

  bool is_color() const;
  bool is_dimension() const;
  bool is_integer() const;
  bool is_keyword() const;
  bool is_number() const;
  bool is_percentage() const;
  ValueType type() const { return type_; }

  // Predicates for keyword value, e.g. is_auto(), is_inherit(), is_none(), etc.
#define V(Name, name) bool is_##name() const;
  FOR_EACH_VISUAL_CSS_KEYWORD_VALUE(V)
#undef V

 private:
  union {
    struct {
      float number;
      Unit unit;
    } dimension;
    uint32_t u32;
    float f32;
  } data_;
  ValueType type_;
};

std::ostream& operator<<(std::ostream& ostream, const Value& value);

}  // namespace css
}  // namespace visuals

#endif  // EVITA_VISUALS_CSS_VALUE_H_
