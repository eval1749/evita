// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_CSS_COLOR_H_
#define EVITA_CSS_COLOR_H_

#include <stdint.h>
#include <functional>

namespace css {

// Color
class Color final {
 public:
  Color(int red, int green, int blue, float alpha = 1.0f);
  Color();
  ~Color();

  bool operator==(const Color& other) const;
  bool operator!=(const Color& other) const;

  float alpha() const { return alpha_; }
  int blue() const { return static_cast<int>(rgb_ & 0xFF); }
  int green() const { return static_cast<int>((rgb_ >> 8) & 0xFF); }
  int red() const { return static_cast<int>((rgb_ >> 16) & 0xFF); }

  bool Equal(const Color& cr) const;
  size_t Hash() const;

 private:
  uint32_t rgb_;
  float alpha_;
};

}  // namespace css

namespace std {
template <>
struct hash<css::Color> {
  size_t operator()(const css::Color& color) const;
};
}  // namespace std

#endif  // EVITA_CSS_COLOR_H_
