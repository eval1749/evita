// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/gfx/color_f.h"

namespace gfx {

ColorF::ColorF(float r, float g, float b, float a)
  : D2D1::ColorF(r, g, b, a) {
}

ColorF::ColorF(D2D1::ColorF::Enum name, float a)
  : D2D1::ColorF(name, a) {
}

ColorF::ColorF(const ColorF& color, float alpha)
  : ColorF(color.r, color.g, color.b, alpha) {
}

ColorF::ColorF(const ColorF& other)
  : ColorF(other.r, other.g, other.b, other.a) {
}

ColorF::ColorF() : ColorF(0.0f, 0.0f, 0.0f, 0.0f) {
}

ColorF::~ColorF() {
}

bool ColorF::operator==(const ColorF& other) const {
  return r == other.r && g == other.g && b == other.b && a == other.a;
}

bool ColorF::operator!=(const ColorF& other) const {
  return !operator==(other);
}

}  // namespace gfx

namespace std {
size_t hash<gfx::ColorF>::operator()(const gfx::ColorF& color) const {
  auto result = std::hash<float>()(color.r);
  result ^= std::hash<float>()(color.g);
  result ^= std::hash<float>()(color.b);
  result ^= std::hash<float>()(color.a);
  return result;
}
}  // namespace std
