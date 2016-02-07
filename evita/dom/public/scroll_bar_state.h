// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_PUBLIC_SCROLL_BAR_STATE_H_
#define EVITA_DOM_PUBLIC_SCROLL_BAR_STATE_H_

#include <iosfwd>

namespace domapi {

#define FOR_EACH_SCROLL_BAR_STATE(V) \
  V(Active)                          \
  V(Disabled)                        \
  V(Hovered)                         \
  V(Normal)                          \
  V(Pressed)

enum class ScrollBarState {
#define V(Name) Name,
  FOR_EACH_SCROLL_BAR_STATE(V)
#undef V
};

std::ostream& operator<<(std::ostream& ostream, ScrollBarState state);

}  // namespace domapi

#endif  // EVITA_DOM_PUBLIC_SCROLL_BAR_STATE_H_
