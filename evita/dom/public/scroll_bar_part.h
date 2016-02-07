// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_PUBLIC_SCROLL_BAR_PART_H_
#define EVITA_DOM_PUBLIC_SCROLL_BAR_PART_H_

#include <iosfwd>

namespace domapi {

#define FOR_EACH_SCROLL_BAR_PART(V) \
  V(BackwardButton)                 \
  V(BackwardTrack)                  \
  V(BlankCorner)                    \
  V(ForwardButton)                  \
  V(ForwardTrack)                   \
  V(None)                           \
  V(Thumb)                          \
  V(ResizeCorner)

enum class ScrollBarPart {
#define V(Name) Name,
  FOR_EACH_SCROLL_BAR_PART(V)
#undef V
};

std::ostream& operator<<(std::ostream& ostream, ScrollBarPart part);

}  // namespace domapi

#endif  // EVITA_DOM_PUBLIC_SCROLL_BAR_PART_H_
