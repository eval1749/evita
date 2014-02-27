// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/css/color.h"

namespace css {

Color::Color(COLORREF cr) : m_cr(cr) {
}

Color::Color(int r, int g, int b) : m_cr(RGB(r, g, b)) {
}

Color::~Color() {
}

bool Color::operator==(const Color& other) const {
  return m_cr == other.m_cr;
}

bool Color::operator!=(const Color& other) const {
  return !operator==(other);
}

}  // namespace css

namespace std {
size_t hash<css::Color>::operator()(const css::Color& color) const {
  return color.Hash();
}
}  // namespace std