// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_STYLE_FLOAT_COLOR_H_
#define EVITA_VISUALS_STYLE_FLOAT_COLOR_H_

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// FloatColor
//
class FloatColor final {
 public:
  FloatColor(float red, float green, float blue, float alpha = 1);
  FloatColor(const FloatColor& other);
  FloatColor();
  ~FloatColor();

  bool operator==(const FloatColor& other) const;
  bool operator!=(const FloatColor& other) const;

  float blue() const { return blue_; }
  float green() const { return green_; }
  float red() const { return red_; }

 private:
  float alpha_ = 0;
  float blue_ = 0;
  float green_ = 0;
  float red_ = 0;
};

}  // namespace visuals

#endif  // EVITA_VISUALS_STYLE_FLOAT_COLOR_H_
