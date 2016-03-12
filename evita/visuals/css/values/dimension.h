// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_CSS_VALUES_DIMENSION_H_
#define EVITA_VISUALS_CSS_VALUES_DIMENSION_H_

#include <stdint.h>

#include <iosfwd>

namespace visuals {
namespace css {

enum class Unit : uint32_t;

//////////////////////////////////////////////////////////////////////
//
// Dimension
//
class Dimension {
 public:
  // TODO(eval1749): We should make |Dimension| constructors protected.
  Dimension(float number, Unit unit);
  Dimension(const Dimension& other);
  ~Dimension();

  Dimension& operator=(const Dimension& other);

  bool operator==(const Dimension& other) const;
  bool operator!=(const Dimension& other) const;

  bool is_absolute_length() const;
  bool is_angle() const;
  bool is_duration() const;
  bool is_font_relative_length() const;
  bool is_frequency() const;
  bool is_length() const;
  bool is_resolution() const;
  bool is_viewport_percentage_length() const;

  float number() const { return number_; }
  Unit unit() const { return unit_; }

  static bool is_absolute_length(Unit unit);
  static bool is_angle(Unit unit);
  static bool is_duration(Unit unit);
  static bool is_font_relative_length(Unit unit);
  static bool is_frequency(Unit unit);
  static bool is_length(Unit unit);
  static bool is_resolution(Unit unit);
  static bool is_viewport_percentage_length(Unit unit);

 private:
  float number_;
  Unit unit_;
};

std::ostream& operator<<(std::ostream& ostream, const Dimension& dimension);

}  // namespace css
}  // namespace visuals

#endif  // EVITA_VISUALS_CSS_VALUES_DIMENSION_H_
