// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_css_color_h)
#define INCLUDE_evita_css_color_h

#include <functional>

namespace css {

// Color
class Color {
  COLORREF    m_cr;

  public: Color(COLORREF cr = 0);
  public: Color(int r, int g, int b);
  public: ~Color();

  public: operator COLORREF() const { return m_cr; }

  public: bool operator==(const Color& other) const;
  public: bool operator!=(const Color& other) const;

  public: bool Equal(const Color& cr) const { return m_cr == cr.m_cr; }
  public: size_t Hash() const { return m_cr; }
};

}  // namespace css

namespace std {
template<> struct hash<css::Color> {
  size_t operator()(const css::Color& color) const;
};
}  // namespace std

#endif //!defined(INCLUDE_evita_css_color_h)
