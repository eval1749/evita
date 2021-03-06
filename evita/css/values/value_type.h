// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_CSS_VALUES_VALUE_TYPE_H_
#define EVITA_CSS_VALUES_VALUE_TYPE_H_

#include <stdint.h>

#include <iosfwd>

namespace css {

class ColorValue;
class Dimension;
class Percentage;
enum class Unit : uint32_t;

#define FOR_EACH_CSS_VALUE_TYPE(name) \
  V(Unspecified)                      \
  V(Color)                            \
  V(Dimension)                        \
  V(Integer)                          \
  V(Keyword)                          \
  V(Number)                           \
  V(Percentage)                       \
  V(String)

//////////////////////////////////////////////////////////////////////
//
// ValueType
//
enum class ValueType : uint32_t {
#define V(name) name,
  FOR_EACH_CSS_VALUE_TYPE(V)
#undef V
};

std::ostream& operator<<(std::ostream& ostream, ValueType type);

}  // namespace css

#endif  // EVITA_CSS_VALUES_VALUE_TYPE_H_
