// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_LAYOUT_SCROLL_BAR_H_
#define EVITA_LAYOUT_SCROLL_BAR_H_

#include <memory>
#include <vector>

#include "base/macros.h"
#include "evita/base/float_range.h"
#include "evita/dom/public/scroll_bar_data.h"
#include "evita/gfx/rect_f.h"

namespace domapi {
class ScrollBarData;
enum class ScrollBarOrientation;
enum class ScrollBarPart;
enum class ScrollBarState;
}

namespace visuals {
class DisplayItemList;
}

namespace layout {

using ScrollBarData = domapi::ScrollBarData;
using ScrollBarOrientation = domapi::ScrollBarOrientation;
using ScrollBarPart = domapi::ScrollBarPart;
using ScrollBarState = domapi::ScrollBarState;

//////////////////////////////////////////////////////////////////////
//
// ScrollBar
//
class ScrollBar final {
  using DisplayItemList = visuals::DisplayItemList;

 public:
  class Part;

  explicit ScrollBar(ScrollBarOrientation orientation);
  ~ScrollBar();

  const gfx::RectF& bounds() const { return bounds_; }

  ScrollBarPart HitTestPoint(const gfx::PointF& point) const;
  bool IsVertical() const;
  std::unique_ptr<DisplayItemList> Paint();
  void SetBounds(const gfx::RectF& new_bounds);
  void SetData(const ScrollBarData& new_data);
  void SetState(ScrollBarPart part, ScrollBarState new_state);
  ScrollBarState StateOf(ScrollBarPart part) const;

 private:
  Part* FindPart(ScrollBarPart part) const;
  void UpdateLayoutIfNeeded();

  gfx::RectF bounds_;
  ScrollBarData data_;
  const std::vector<Part*> parts_;
  const ScrollBarOrientation orientation_;

  DISALLOW_COPY_AND_ASSIGN(ScrollBar);
};

}  // namespace layout

#endif  // EVITA_LAYOUT_SCROLL_BAR_H_
