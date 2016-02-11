// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_WINDOWS_SCROLL_BAR_H_
#define EVITA_DOM_WINDOWS_SCROLL_BAR_H_

#include <memory>

#include "base/macros.h"
#include "evita/base/float_range.h"
#include "evita/dom/public/float_point.h"
#include "evita/dom/public/float_rect.h"
#include "evita/dom/public/scroll_bar_data.h"
#include "evita/ui/base/repeat_controller.h"

namespace domapi {
struct MouseEvent;
enum class ScrollBarOrientation;
enum class ScrollBarPart;
enum class ScrollBarState;
}

namespace layout {
class ScrollBar;
}

namespace ui {
class ScrollBarObserver;
}

namespace visuals {
class DisplayItemList;
}

namespace dom {

class ScrollBarOwner;
class Window;

//////////////////////////////////////////////////////////////////////
//
// ScrollBar
//
class ScrollBar final {
  using DisplayItemList = visuals::DisplayItemList;
  using FloatPoint = domapi::FloatPoint;
  using FloatRect = domapi::FloatRect;
  using MouseEvent = domapi::MouseEvent;
  using ScrollBarData = domapi::ScrollBarData;
  using ScrollBarOrientation = domapi::ScrollBarOrientation;
  using ScrollBarPart = domapi::ScrollBarPart;
  using ScrollBarState = domapi::ScrollBarState;

 public:
  ScrollBar(ScrollBarOrientation orientation,
            ScrollBarOwner* owner,
            ui::ScrollBarObserver* observer);
  ~ScrollBar();

  // Returns true if event handled.
  bool HandleMouseEvent(const MouseEvent& event);
  ScrollBarPart HitTestPoint(const FloatPoint& point) const;
  std::unique_ptr<DisplayItemList> Paint() const;
  void SetBounds(const FloatRect& bounds);
  void SetData(const ScrollBarData& new_data);
  void SetDisabled(bool new_disabled);

 private:
  void DidFireRepeatTimer();
  bool HandleMouseMoved(const MouseEvent& event);
  bool HandleMousePressed(const MouseEvent& event);
  bool HandleMouseReleased(const MouseEvent& event);
  bool IsDisabled(ScrollBarPart part) const;
  void UpdateHoveredPart(ScrollBarPart part);

  ScrollBarPart active_part_;
  ScrollBarData data_;
  ScrollBarPart hovered_part_;
  bool disabled_ = false;
  FloatPoint last_drag_point_;
  std::unique_ptr<layout::ScrollBar> layout_;
  ui::ScrollBarObserver* const observer_;
  ScrollBarOwner* const owner_;
  ui::RepeatController repeat_controller_;

  DISALLOW_COPY_AND_ASSIGN(ScrollBar);
};

//////////////////////////////////////////////////////////////////////
//
// ScrollBarOwner
//
class ScrollBarOwner {
 public:
  virtual ~ScrollBarOwner();

  virtual void DidChangeScrollBar() = 0;
  virtual void DidReleaseScrollBar() = 0;
  virtual void DidPressScrollBar() = 0;

 protected:
  ScrollBarOwner();

 private:
  DISALLOW_COPY_AND_ASSIGN(ScrollBarOwner);
};

}  // namespace dom

#endif  // EVITA_DOM_WINDOWS_SCROLL_BAR_H_
