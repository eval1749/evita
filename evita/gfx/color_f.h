// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_GFX_COLOR_F_H_
#define EVITA_GFX_COLOR_F_H_

#include <functional>

#include "evita/gfx/forward.h"

namespace gfx {

class ColorF final : public D2D1::ColorF {
 public:
  ColorF(float r, float g, float b, float a = 1.0f);
  explicit ColorF(D2D1::ColorF::Enum name, float a = 1.0f);
  ColorF(const ColorF& other, float a);
  ColorF(const ColorF& other);
  ColorF();
  ~ColorF();

  bool operator==(const ColorF& other) const;
  bool operator!=(const ColorF& other) const;

  float alpha() const { return a; }
  float blue() const { return b; }
  float green() const { return g; }
  float red() const { return r; }
};

// Helper functions
inline ColorF blackColor() {
  return ColorF(ColorF::Black);
}

inline ColorF grayColor() {
  return ColorF(ColorF::LightGray);
}

inline ColorF sysColor(int name, float alpha = 1) {
  auto const colorRef = ::GetSysColor(name);
  return ColorF(static_cast<float>(GetRValue(colorRef)) / 255,
                static_cast<float>(GetGValue(colorRef)) / 255,
                static_cast<float>(GetBValue(colorRef)) / 255, alpha);
}

inline ColorF whiteColor() {
  return ColorF(ColorF::White);
}

}  // namespace gfx

namespace std {
template <>
struct hash<gfx::ColorF> {
  size_t operator()(const gfx::ColorF& color) const;
};
}  // namespace std

#endif  // EVITA_GFX_COLOR_F_H_
