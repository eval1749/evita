// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_PAINT_SCROLL_BAR_PAINTER_H_
#define EVITA_PAINT_SCROLL_BAR_PAINTER_H_

#include "base/macros.h"

namespace domapi {
enum class ScrollBarState;
}

namespace visuals {
class DisplayItemListBuilder;
class FloatRect;
}

namespace paint {

//////////////////////////////////////////////////////////////////////
//
// ScrollBarPainter
//
class ScrollBarPainter final {
  using DisplayItemListBuilder = visuals::DisplayItemListBuilder;
  using FloatRect = visuals::FloatRect;
  using ScrollBarState = domapi::ScrollBarState;

 public:
  ScrollBarPainter();
  ~ScrollBarPainter();

  void PaintBottomButton(DisplayItemListBuilder* builder,
                         const FloatRect& bounds,
                         ScrollBarState state);

  void PaintBottomTrack(DisplayItemListBuilder* builder,
                        const FloatRect& bounds,
                        ScrollBarState state);

  void PaintHorizontalThumb(DisplayItemListBuilder* builder,
                            const FloatRect& bounds,
                            ScrollBarState state);

  void PaintLeftButton(DisplayItemListBuilder* builder,
                       const FloatRect& bounds,
                       ScrollBarState state);

  void PaintLeftTrack(DisplayItemListBuilder* builder,
                      const FloatRect& bounds,
                      ScrollBarState state);

  void PaintRightButton(DisplayItemListBuilder* builder,
                        const FloatRect& bounds,
                        ScrollBarState state);

  void PaintRightTrack(DisplayItemListBuilder* builder,
                       const FloatRect& bounds,
                       ScrollBarState state);

  void PaintTopButton(DisplayItemListBuilder* builder,
                      const FloatRect& bounds,
                      ScrollBarState state);

  void PaintTopTrack(DisplayItemListBuilder* builder,
                     const FloatRect& bounds,
                     ScrollBarState state);

  void PaintVerticalThumb(DisplayItemListBuilder* builder,
                          const FloatRect& bounds,
                          ScrollBarState state);

 private:
  DISALLOW_COPY_AND_ASSIGN(ScrollBarPainter);
};

}  // namespace paint

#endif  // EVITA_PAINT_SCROLL_BAR_PAINTER_H_
