// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_CSS_UNIT_H_
#define EVITA_VISUALS_CSS_UNIT_H_

#include <iosfwd>

namespace visuals {
namespace css {

#define FOR_EACH_CSS_UNIT(V)        \
  /* Font-relative lengths */       \
  V(em)                             \
  V(ex)                             \
  V(ch)                             \
  V(rem)                            \
  /* Viewport-percentage lengths */ \
  V(vw)                             \
  V(vh)                             \
  V(vmin)                           \
  V(vmax)                           \
  /* Absolute lengths */            \
  V(cm)                             \
  V(mm)                             \
  V(q)                              \
  V(in)                             \
  V(pt)                             \
  V(pc)                             \
  V(px)                             \
  /* Angle units */                 \
  V(deg)                            \
  V(grad)                           \
  V(rad)                            \
  V(turn)                           \
  /* Duration units */              \
  V(s)                              \
  V(ms)                             \
  /* Frequency units */             \
  V(Hz)                             \
  V(kHz)                            \
  /* Resolution units */            \
  V(dpi)                            \
  V(dpcm)                           \
  V(dppx)

//////////////////////////////////////////////////////////////////////
//
// Unit
//
enum class Unit {
#define V(name) name,
  FOR_EACH_CSS_UNIT(V)
#undef V
};

std::ostream& operator<<(std::ostream& ostream, Unit unit);

}  // namespace css
}  // namespace visuals

#endif  // EVITA_VISUALS_CSS_UNIT_H_
