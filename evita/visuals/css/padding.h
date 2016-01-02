// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_CSS_PADDING_H_
#define EVITA_VISUALS_CSS_PADDING_H_

#include <iosfwd>

#include "evita/visuals/css/thickness.h"

namespace visuals {
namespace css {

//////////////////////////////////////////////////////////////////////
//
// Padding
//
// padding: top, right, bottom, left
// padding: top, right_left, bottom
// padding: top-bottom, right_left
// padding: top-bottom-right-left
//
class Padding final {
 public:
  Padding(float top, float right, float bottom, float left);
  Padding(float top, float right_left, float bottom);
  Padding(float top_bottom, float right_left);
  explicit Padding(float width);
  Padding(const Padding& other);
  Padding();
  ~Padding();

  bool operator==(const Padding& other) const;
  bool operator!=(const Padding& other) const;

  float bottom() const { return thickness_.bottom(); }
  FloatSize bottom_right() const { return thickness_.bottom_right(); }
  float left() const { return thickness_.left(); }
  float right() const { return thickness_.right(); }
  FloatSize size() const { return thickness_.size(); }
  float top() const { return thickness_.top(); }
  FloatSize top_left() const { return thickness_.top_left(); }

  bool HasValue() const { return thickness_.HasValue(); }

 private:
  Thickness thickness_;
};

std::ostream& operator<<(std::ostream& ostream, const Padding& padding);

}  // namespace css
}  // namespace visuals

#endif  // EVITA_VISUALS_CSS_PADDING_H_
