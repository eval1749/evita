// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_PUBLIC_CURSOR_H_
#define EVITA_DOM_PUBLIC_CURSOR_H_

#include <iosfwd>

namespace domapi {

#define FOR_EACH_DOMAPI_CURSOR(V) \
  V(Alias)                        \
  V(Cell)                         \
  V(ColumnResize)                 \
  V(ContextMenu)                  \
  V(Copy)                         \
  V(Cross)                        \
  V(Custom)                       \
  V(EastPanning)                  \
  V(EastResize)                   \
  V(EastWestResize)               \
  V(Grab)                         \
  V(Grabbing)                     \
  V(Hand)                         \
  V(Help)                         \
  V(IBeam)                        \
  V(MiddlePanning)                \
  V(Move)                         \
  V(NoDrop)                       \
  V(None)                         \
  V(NorthEastPanning)             \
  V(NorthEastResize)              \
  V(NorthEastSouthWestResize)     \
  V(NorthPanning)                 \
  V(NorthResize)                  \
  V(NorthSouthResize)             \
  V(NorthWestPanning)             \
  V(NorthWestResize)              \
  V(NorthWestSouthEastResize)     \
  V(NotAllowed)                   \
  V(Pointer)                      \
  V(Progress)                     \
  V(RowResize)                    \
  V(SouthEastPanning)             \
  V(SouthEastResize)              \
  V(SouthPanning)                 \
  V(SouthResize)                  \
  V(SouthWestPanning)             \
  V(SouthWestResize)              \
  V(VerticalText)                 \
  V(Wait)                         \
  V(WestPanning)                  \
  V(WestResize)                   \
  V(ZoomIn)                       \
  V(ZoomOut)

enum class CursorId {
#define V(Name) Name,
  FOR_EACH_DOMAPI_CURSOR(V)
#undef V
};

std::ostream& operator<<(std::ostream& ostream, CursorId cursor_id);

}  // namespace domapi

#endif  // EVITA_DOM_PUBLIC_CURSOR_H_
