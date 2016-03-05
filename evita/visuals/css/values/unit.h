// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_CSS_VALUES_UNIT_H_
#define EVITA_VISUALS_CSS_VALUES_UNIT_H_

#include <iosfwd>

namespace visuals {
namespace css {

#define FOR_EACH_CSS_ABSOLUTE_LENGTH(V) \
  V(cm)                                 \
  V(mm)                                 \
  V(in)                                 \
  V(pt)                                 \
  V(pc)                                 \
  V(px)                                 \
  V(q)

#define FOR_EACH_CSS_ANGLE_UNIT(V) \
  V(deg)                           \
  V(grad)                          \
  V(rad)                           \
  V(turn)

#define FOR_EACH_CSS_DURATION_UNIT(V) \
  V(ms)                               \
  V(s)

#define FOR_EACH_CSS_FONT_RELATIVE_LENGTH(V) \
  V(ch)                                      \
  V(em)                                      \
  V(ex)                                      \
  V(rem)

#define FOR_EACH_CSS_FREQUENCY_UNIT(V) \
  V(Hz)                                \
  V(kHz)

#define FOR_EACH_CSS_RESOLUTION_UNIT(V) \
  V(dpi)                                \
  V(dpcm)                               \
  V(dppx)

#define FOR_EACH_CSS_VIEWPORT_PERCENTAGE_LENGTH(V) \
  V(vh)                                            \
  V(vmin)                                          \
  V(vmax)                                          \
  V(vw)

#define FOR_EACH_CSS_UNIT(V)           \
  FOR_EACH_CSS_ABSOLUTE_LENGTH(V)      \
  FOR_EACH_CSS_ANGLE_UNIT(V)           \
  FOR_EACH_CSS_DURATION_UNIT(V)        \
  FOR_EACH_CSS_FONT_RELATIVE_LENGTH(V) \
  FOR_EACH_CSS_FREQUENCY_UNIT(V)       \
  FOR_EACH_CSS_RESOLUTION_UNIT(V)      \
  FOR_EACH_CSS_VIEWPORT_PERCENTAGE_LENGTH(V)

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

#endif  // EVITA_VISUALS_CSS_VALUES_UNIT_H_
