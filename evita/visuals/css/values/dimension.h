// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_CSS_VALUES_DIMENSION_H_
#define EVITA_VISUALS_CSS_VALUES_DIMENSION_H_

#include <iosfwd>

namespace visuals {
namespace css {

enum class Unit;

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

 private:
  float number_;
  Unit unit_;
};

std::ostream& operator<<(std::ostream& ostream, const Dimension& dimension);

}  // namespace css
}  // namespace visuals

#endif  // EVITA_VISUALS_CSS_VALUES_DIMENSION_H_
