// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_STYLE_DISPLAY_H_
#define EVITA_VISUALS_STYLE_DISPLAY_H_

#include <iosfwd>

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// Display
//
#define FOR_EACH_VISUALS_ENUM_DISPLAY(V) \
  V(None)                                \
  V(Block)                               \
  V(Inline)                              \
  V(InlineBlock)

enum class Display {
#define V(name) name,
  FOR_EACH_VISUALS_ENUM_DISPLAY(V)
#undef V
};

std::ostream& operator<<(std::ostream& ostream, Display);

}  // namespace visuals

#endif  // EVITA_VISUALS_STYLE_DISPLAY_H_
