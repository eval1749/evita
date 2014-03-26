// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUE_evita_gfx_color_f_h)
#define INCLUE_evita_gfx_color_f_h

#include <functional>

#include "evita/gfx/forward.h"

namespace gfx {

class ColorF : public D2D1::ColorF {
  public: ColorF(float r, float g, float b, float a = 1.0f);
  public: ColorF(D2D1::ColorF::Enum name, float a = 1.0f);
  public: ColorF(const ColorF& other, float a);
  public: ColorF(const ColorF& other);
  public: ColorF();
  public: ~ColorF();

  public: bool operator==(const ColorF& other) const;
  public: bool operator!=(const ColorF& other) const;

  public: float alpha() const { return a; }
  public: float blue() const { return b; }
  public: float green() const { return g; }
  public: float red() const { return r; }
};

} // namespace gfx

namespace std {
template<> struct hash<gfx::ColorF> {
  size_t operator()(const gfx::ColorF& color) const;
};
}  // namespace std


#endif //!defined(INCLUDE_evita_gfx_color_f_h)
