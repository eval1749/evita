// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_CSS_BORDER_H_
#define EVITA_VISUALS_CSS_BORDER_H_

#include <iosfwd>

#include "evita/visuals/css/color.h"
#include "evita/visuals/css/thickness.h"

namespace visuals {
namespace css {

//////////////////////////////////////////////////////////////////////
//
// Border
//
class Border final {
 public:
  Border(const Color& color, float top, float right, float bottom, float left);
  Border(const Color& color, float width);
  Border(const Border& other);
  Border();
  ~Border();

  bool operator==(const Border& other) const;
  bool operator!=(const Border& other) const;

  // Sizes
  float bottom() const { return thickness_.bottom(); }
  float left() const { return thickness_.left(); }
  float right() const { return thickness_.right(); }
  float top() const { return thickness_.top(); }

  FloatSize bottom_right() const { return thickness_.bottom_right(); }
  FloatSize size() const { return thickness_.size(); }
  FloatSize top_left() const { return thickness_.top_left(); }

  // Colors
  const Color& bottom_color() const { return bottom_color_; }
  const Color& left_color() const { return left_color_; }
  const Color& right_color() const { return right_color_; }
  const Color& top_color() const { return top_color_; }

  bool HasValue() const;
  bool IsSimple() const;

 private:
  Color bottom_color_;
  Color left_color_;
  Color right_color_;
  Color top_color_;
  Thickness thickness_;
};

std::ostream& operator<<(std::ostream& ostream, const Border& border);

}  // namespace css
}  // namespace visuals

#endif  // EVITA_VISUALS_CSS_BORDER_H_
