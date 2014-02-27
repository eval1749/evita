// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_css_color_h)
#define INCLUDE_evita_css_color_h

#include <functional>

#include "base/basictypes.h"

namespace css {

// Color
class Color {
  private: uint32_t rgb_;
  private: float alpha_;

  public: Color(int red, int green, int blue, float alpha = 1.0f);
  public: Color();
  public: ~Color();

  public: bool operator==(const Color& other) const;
  public: bool operator!=(const Color& other) const;

  public: float alpha() const { return alpha_; }
  public: int blue() const { return static_cast<int>(rgb_ & 0xFF); }
  public: int green() const { return static_cast<int>((rgb_ >> 8) & 0xFF); }
  public: int red() const { return static_cast<int>((rgb_ >> 16) & 0xFF); }

  public: bool Equal(const Color& cr) const;
  public: size_t Hash() const;
};

}  // namespace css

namespace std {
template<> struct hash<css::Color> {
  size_t operator()(const css::Color& color) const;
};
}  // namespace std

#endif //!defined(INCLUDE_evita_css_color_h)
