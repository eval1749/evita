// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_CSS_PROPERTY_SET_H_
#define EVITA_VISUALS_CSS_PROPERTY_SET_H_

#include <stdint.h>

#include <iosfwd>
#include <iterator>
#include <utility>
#include <vector>

namespace visuals {
namespace css {

class Color;
class Dimension;
enum class Keyword;
class Percentage;
enum class PropertyId;
enum class Unit;
enum class ValueType : uint32_t;
class Value;

//////////////////////////////////////////////////////////////////////
//
// PropertySet
//
class PropertySet final {
 public:
  // See "evita/visuals/css/property_set_builder.h" for implementation.
  class Builder;
  class Iterator;

  PropertySet(const PropertySet& other);
  PropertySet(PropertySet&& other);
  PropertySet();
  ~PropertySet();

  PropertySet& operator=(const PropertySet& other);
  PropertySet& operator=(PropertySet&& other);

  bool operator==(const PropertySet& other) const;
  bool operator!=(const PropertySet& other) const;

  Iterator begin() const;
  Iterator end() const;

  bool Contains(PropertyId property_id) const;
  Value ValueOf(PropertyId property_id) const;

 private:
  // Color
  //  RGBA value is placed in next word
  //  RGBA(32-bit)
  // Dimension
  //  small_value(14bit) + unit(5bit) + large(1bit)
  // Integer
  //  small_value(19bit) + large(1bit)
  //  int(32bit)
  // Number
  //  small_value(19bit) + large(1bit)
  //  float(32bit)
  // Percentage
  //  small_percentage(19bit) + large(1bit)
  //  float(32bit)

  static const auto kPropertyIdBits = 9;
  static const auto kTypeBits = 3;
  static const auto kDataBits = 20;
  static_assert(kPropertyIdBits + kTypeBits + kDataBits == 32,
                "kPropertyIdBits + kTypeBits + kDataBits == 32");

  union Word {
    struct {
      PropertyId property_id : kPropertyIdBits;
      ValueType type : kTypeBits;
      uint32_t data : kDataBits;
    } bits;
    uint32_t u32;
    float f32;
  };
  static_assert(sizeof(Word) == sizeof(uint32_t), "Word must be 32-bit");

  std::vector<Word> words_;
};

std::ostream& operator<<(std::ostream& ostream,
                         const PropertySet& property_set);

//////////////////////////////////////////////////////////////////////
//
// PropertySet::Iterator
//
class PropertySet::Iterator final {
 public:
  using iterator_category = std::forward_iterator_tag;
  using difference_type = int;
  using value_type = std::pair<PropertyId, Value>;
  using pointer = value_type*;
  using reference = value_type&;

  Iterator(const Iterator& other);
  Iterator(const PropertySet* property_set, size_t index);
  ~Iterator();

  Iterator& operator=(const Iterator& other);

  value_type operator*() const;
  Iterator& operator++();

  bool operator==(const Iterator& other) const;
  bool operator!=(const Iterator& other) const;

 private:
  uint32_t data() const;
  bool is_small() const;
  PropertyId property_id() const;
  ValueType type() const;

  Color DecodeColor() const;
  Dimension DecodeDimension() const;
  float DecodeFloat() const;
  int DecodeInteger() const;
  Keyword DecodeKeyword() const;
  float DecodeNumber() const;
  Percentage DecodePercentage() const;
  int DecodeSmallInteger() const;
  uint32_t DecodeUint32() const;
  Value DecodeValue() const;
  size_t SizeOfChunk() const;

  size_t index_;
  const PropertySet* property_set_;
};

}  // namespace css
}  // namespace visuals

#endif  // EVITA_VISUALS_CSS_PROPERTY_SET_H_
