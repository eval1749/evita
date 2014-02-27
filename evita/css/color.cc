// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/css/color.h"

#include "base/logging.h"

namespace css {

namespace {
uint32_t PackRgbToUint32(int red, int green, int blue) {
  DCHECK_LE(static_cast<uint32_t>(red), 0xFFu);
  DCHECK_LE(static_cast<uint32_t>(green), 0xFFu);
  DCHECK_LE(static_cast<uint32_t>(blue), 0xFFu);
  return (static_cast<uint32_t>(red & 0xFF) << 16) |
         (static_cast<uint32_t>(green & 0xFF) << 8) |
         static_cast<uint32_t>(blue & 0xFF);
}
}  // namespace

Color::Color(int red, int green, int blue, float alpha)
    : alpha_(alpha), rgb_(PackRgbToUint32(red, green, blue)) {
}

Color::Color() : Color(0, 0, 0, 0.0f) {
}

Color::~Color() {
}

bool Color::operator==(const Color& other) const {
  return rgb_ == other.rgb_ && alpha_ == other.alpha_;
}

bool Color::operator!=(const Color& other) const {
  return !operator==(other);
}

size_t Color::Hash() const {
  return std::hash<uint32_t>()(rgb_) ^ std::hash<float>()(alpha_);
}

}  // namespace css

namespace std {
size_t hash<css::Color>::operator()(const css::Color& color) const {
  return color.Hash();
}
}  // namespace std