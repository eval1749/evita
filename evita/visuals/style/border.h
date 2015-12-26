// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_STYLE_BORDER_H_
#define EVITA_VISUALS_STYLE_BORDER_H_

#include <iosfwd>

#include "evita/visuals/style/thickness.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// Border
//
class Border final {
 public:
  Border(float top, float left, float bottom, float right);
  Border(const Border& other);
  Border();
  ~Border();

  bool operator==(const Border& other) const;
  bool operator!=(const Border& other) const;

  float bottom() const { return thickness_.bottom(); }
  FloatSize bottom_right() const { return thickness_.bottom_right(); }
  float left() const { return thickness_.left(); }
  float right() const { return thickness_.right(); }
  FloatSize size() const { return thickness_.size(); }
  float top() const { return thickness_.top(); }
  FloatSize top_left() const { return thickness_.top_left(); }

 private:
  Thickness thickness_;
};

std::ostream& operator<<(std::ostream& ostream, const Border& border);

}  // namespace visuals

#endif  // EVITA_VISUALS_STYLE_BORDER_H_
