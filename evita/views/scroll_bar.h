// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VIEWS_SCROLL_BAR_H_
#define EVITA_VIEWS_SCROLL_BAR_H_

#include <memory>

#include "base/macros.h"
#include "evita/base/float_range.h"
#include "evita/dom/public/scroll_bar_data.h"
#include "evita/gfx/rect_f.h"

namespace domapi {
enum class ScrollBarOrientation;
enum class ScrollBarPart;
enum class ScrollBarState;
}

namespace gfx {
class Canvas;
}

namespace layout {
class ScrollBar;
}

namespace ui {
class MouseEvent;
class ScrollBarObserver;
class Widget;
}

namespace views {

using ScrollBarData = domapi::ScrollBarData;
using ScrollBarOrientation = domapi::ScrollBarOrientation;
using ScrollBarPart = domapi::ScrollBarPart;
using ScrollBarState = domapi::ScrollBarState;

//////////////////////////////////////////////////////////////////////
//
// ScrollBar
//
class ScrollBar final {
 public:
  ScrollBar(ScrollBarOrientation orientation,
            ui::Widget* window,
            ui::ScrollBarObserver* observer);
  ~ScrollBar();

  // Returns true if event handled.
  bool HandleMouseMoved(const ui::MouseEvent& event);
  // Returns true if event handled.
  bool HandleMousePressed(const ui::MouseEvent& event);
  // Returns true if event handled.
  bool HandleMouseReleased(const ui::MouseEvent& event);
  ScrollBarPart HitTestPoint(const gfx::PointF& point) const;
  void Paint(gfx::Canvas* canvas) const;
  void SetBounds(const gfx::RectF& bounds);
  void SetData(const ScrollBarData& new_data);
  void SetDisabled(bool new_disabled);

 private:
  void UpdateHoveredPart(ScrollBarPart part);

  ScrollBarPart active_part_;
  ScrollBarData data_;
  ScrollBarPart hovered_part_;
  bool disabled_ = false;
  gfx::PointF last_drag_point_;
  std::unique_ptr<layout::ScrollBar> layout_;
  ui::ScrollBarObserver* const observer_;
  ui::Widget* const window_;

  DISALLOW_COPY_AND_ASSIGN(ScrollBar);
};

}  // namespace views

#endif  // EVITA_VIEWS_SCROLL_BAR_H_
