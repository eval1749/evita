// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_LAYOUT_BORDER_H_
#define EVITA_VISUALS_LAYOUT_BORDER_H_

#include <iosfwd>

#include "evita/visuals/css/float_color.h"
#include "evita/visuals/layout/thickness.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// Border
//
class Border final {
 public:
  Border(const FloatColor& top_color,
         float top,
         const FloatColor& right_color,
         float right,
         const FloatColor& bottom_color,
         float bottom,
         const FloatColor& left_color,
         float left);
  Border(const FloatColor& color,
         float top,
         float right,
         float bottom,
         float left);
  Border(const FloatColor& color, float width);
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
  const FloatColor& bottom_color() const { return bottom_color_; }
  const FloatColor& left_color() const { return left_color_; }
  const FloatColor& right_color() const { return right_color_; }
  const FloatColor& top_color() const { return top_color_; }

  bool HasValue() const;
  bool IsSimple() const;

 private:
  FloatColor bottom_color_;
  FloatColor left_color_;
  FloatColor right_color_;
  FloatColor top_color_;
  Thickness thickness_;
};

std::ostream& operator<<(std::ostream& ostream, const Border& border);

}  // namespace visuals

#endif  // EVITA_VISUALS_LAYOUT_BORDER_H_
