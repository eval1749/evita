// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_STYLE_MARGIN_H_
#define EVITA_VISUALS_STYLE_MARGIN_H_

#include <iosfwd>

#include "evita/visuals/style/thickness.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// Margin
//
// Thickness can have negative values to make adjacent boxes overlap each
// other.
//
class Margin final {
 public:
  Margin(float top, float left, float bottom, float right);
  Margin(const Margin& other);
  Margin();
  ~Margin();

  bool operator==(const Margin& other) const;
  bool operator!=(const Margin& other) const;

  float bottom() const { return thickness_.bottom(); }
  FloatSize bottom_right() const { return thickness_.bottom_right(); }
  float left() const { return thickness_.left(); }
  float right() const { return thickness_.right(); }
  float top() const { return thickness_.top(); }
  FloatSize top_left() const { return thickness_.top_left(); }

 private:
  Thickness thickness_;
};

std::ostream& operator<<(std::ostream& ostream, const Margin& margin);

}  // namespace visuals

#endif  // EVITA_VISUALS_STYLE_MARGIN_H_
