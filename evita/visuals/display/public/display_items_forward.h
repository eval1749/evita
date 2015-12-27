// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_DISPLAY_PUBLIC_DISPLAY_ITEMS_FORWARD_H_
#define EVITA_VISUALS_DISPLAY_PUBLIC_DISPLAY_ITEMS_FORWARD_H_

namespace visuals {

#define FOR_EACH_DISPLAY_ITEM(V) \
  V(BeginClip)                   \
  V(DrawRect)                    \
  V(DrawText)                    \
  V(EndClip)                     \
  V(FillRect)

#define FOR_EACH_ABSTRACT_DISPLAY_ITEM(V) V(DisplayItem)

// Forward declarations
#define V(name) class name##DisplayItem;
FOR_EACH_DISPLAY_ITEM(V)
FOR_EACH_ABSTRACT_DISPLAY_ITEM(V)
#undef V

class DisplayItem;
class DisplayItemVisitor;

}  // namespace visuals

#endif  // EVITA_VISUALS_DISPLAY_PUBLIC_DISPLAY_ITEMS_FORWARD_H_
