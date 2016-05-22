// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_CSS_VALUES_PERCENTAGE_H_
#define EVITA_CSS_VALUES_PERCENTAGE_H_

#include <iosfwd>

namespace css {

//////////////////////////////////////////////////////////////////////
//
// Percentage
//
class Percentage final {
 public:
  explicit Percentage(float value);
  Percentage(const Percentage& other);
  Percentage();
  ~Percentage();

  bool operator==(const Percentage& other) const;
  bool operator!=(const Percentage& other) const;

  float value() const { return value_; }

 private:
  float value_ = 0;
};

std::ostream& operator<<(std::ostream& ostream, const Percentage& percentage);

}  // namespace css

#endif  // EVITA_CSS_VALUES_PERCENTAGE_H_
