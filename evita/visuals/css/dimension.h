// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_CSS_DIMENSION_H_
#define EVITA_VISUALS_CSS_DIMENSION_H_

#include <iosfwd>

namespace visuals {
namespace css {

enum class Unit;

//////////////////////////////////////////////////////////////////////
//
// Dimension
//
class Dimension final {
 public:
  Dimension(float number, Unit unit);
  Dimension(const Dimension& other);
  ~Dimension();

  Dimension& operator=(const Dimension& other);

  bool operator==(const Dimension& other) const;
  bool operator!=(const Dimension& other) const;

  float number() const { return number_; }
  Unit unit() const { return unit_; }

 private:
  float number_;
  Unit unit_;
};

std::ostream& operator<<(std::ostream& ostream, const Dimension& dimension);

}  // namespace css
}  // namespace visuals

#endif  // EVITA_VISUALS_CSS_DIMENSION_H_
