// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_CSS_VALUES_VALUE_H_
#define EVITA_VISUALS_CSS_VALUES_VALUE_H_

#include <stddef.h>
#include <stdint.h>

#include <iosfwd>
#include <string>

#include "base/strings/string16.h"
#include "base/strings/string_piece.h"
#include "build/build_config.h"
#include "evita/visuals/css/values/ref_counted_string.h"
#include "evita/visuals/css/values_forward.h"

namespace visuals {
namespace css {

class ColorValue;
class Dimension;
enum class Keyword;
class Length;
class Percentage;
class RefCountedString;
enum class Unit : uint32_t;
enum class ValueType : uint32_t;

//////////////////////////////////////////////////////////////////////
//
// Value
//
class Value final {
 public:
  explicit Value(const ColorValue& color);
  explicit Value(const Dimension& dimension);
  explicit Value(Keyword keyword);
  explicit Value(const Percentage& percentage);
  explicit Value(base::StringPiece16 string);
  explicit Value(const String& string);
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

  ColorValue as_color() const;
  Dimension as_dimension() const;
  int as_integer() const;
  Keyword as_keyword() const;
  float as_number() const;
  Length as_length() const;
  Percentage as_percentage() const;
  String as_string() const;

  bool is_color() const;
  bool is_dimension() const;
  bool is_integer() const;
  bool is_keyword() const;
  bool is_length() const;
  bool is_number() const;
  bool is_percentage() const;
  bool is_string() const;
  bool is_unspecified() const;

  const base::string16& string_value() const;
  ValueType type() const;

  // For ease of "values.h" generator
  ColorValue as_color_value() const;
  bool is_color_value() const { return is_color(); }

// Predicates for keyword value, e.g. is_auto(), is_inherit(), is_none(), etc.
#define V(Name, name) bool is_##name() const;
  FOR_EACH_VISUAL_CSS_KEYWORD_VALUE(V)
#undef V

  void Reset();
  base::string16 ToString16() const;

 private:
#if defined(ARCH_CPU_64_BITS)
  static const size_t kTagBits = 3;
#else
  static const size_t kTagBits = 2;
#endif

  enum class Tag : uint32_t {
    Pointer,
    Immediate,
  };

  union PackedData {
    uint32_t u32;
    float f32;
  };

  struct Packed {
#if defined(ARCH_CPU_LITTLE_ENDIAN)
    Tag tag : kTagBits;
    ValueType type : 8 - kTagBits;
    Unit unit : 5;
    PackedData data;
#else
    PackedData data;
    Unit unit : 5;
    ValueType type : 8 - kTagBits;
    Tag tag : kTagBits;
#endif
  };

  static_assert(sizeof(Packed) <= sizeof(uint64_t),
                "sizeof(Packed) <= sizeof(uint64_t)");

  bool is_immediate() const;

  void DidMove();

  union {
    union Immediate {
      Packed packed;
      uint64_t u64;
    } immediate;
    RefCountedString* string;
  } data_;
};

static_assert(sizeof(Value) == sizeof(uint64_t),
              "sizeof(Value) == sizeof(uint64_t)");

std::ostream& operator<<(std::ostream& ostream, const Value& value);

}  // namespace css
}  // namespace visuals

#endif  // EVITA_VISUALS_CSS_VALUES_VALUE_H_
