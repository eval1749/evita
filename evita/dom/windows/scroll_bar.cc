// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <cmath>

#include "evita/dom/windows/scroll_bar.h"

#include "evita/dom/events/mouse_event.h"
#include "evita/dom/public/scroll_bar_orientation.h"
#include "evita/dom/public/scroll_bar_part.h"
#include "evita/dom/public/scroll_bar_state.h"
#include "evita/dom/public/view_events.h"
#include "evita/dom/windows/window.h"
#include "evita/gfx/canvas.h"
#include "evita/gfx/color_f.h"
#include "evita/layout/scroll_bar.h"
#include "evita/ui/controls/scroll_bar_observer.h"
#include "evita/visuals/display/public/display_item_list.h"

namespace dom {

using DisplayItemList = visuals::DisplayItemList;
using FloatPoint = domapi::FloatPoint;
using FloatRect = domapi::FloatRect;
using FloatSize = domapi::FloatSize;
using MouseButton = domapi::MouseButton;
using ScrollBarPart = domapi::ScrollBarPart;

namespace {

gfx::PointF ToPointF(const FloatPoint& point) {
  return gfx::PointF(point.x(), point.y());
}

gfx::SizeF ToSizeF(const FloatSize& size) {
  return gfx::SizeF(size.width(), size.height());
}

gfx::RectF ToRectF(const FloatRect& rect) {
  return gfx::RectF(ToPointF(rect.origin()), ToSizeF(rect.size()));
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// ScrollBar
//
ScrollBar::ScrollBar(ScrollBarOrientation orientation,
                     Window* window,
                     ui::ScrollBarObserver* observer)
    : active_part_(ScrollBarPart::None),
      hovered_part_(ScrollBarPart::None),
      layout_(new layout::ScrollBar(orientation)),
      observer_(observer),
      window_(window) {
  SetDisabled(true);
}

ScrollBar::~ScrollBar() {}

bool ScrollBar::HandleMouseEvent(const MouseEvent& event) {
  switch (event.event_type) {
    case domapi::EventType::MouseDown:
      return HandleMousePressed(event);
    case domapi::EventType::MouseMove:
      return HandleMouseMoved(event);
    case domapi::EventType::MouseUp:
      return HandleMouseReleased(event);
  }
  return false;
}

bool ScrollBar::HandleMouseMoved(const MouseEvent& event) {
  const auto& point = FloatPoint(event.client_x, event.client_y);
  const auto part = HitTestPoint(point);
  if (active_part_ != ScrollBarPart::Thumb) {
    UpdateHoveredPart(part);
    return part != ScrollBarPart::None;
  }
  if (IsDisabled(part))
    return true;
  const auto delta = layout_->IsVertical() ? point.y() - last_drag_point_.y()
                                           : point.x() - last_drag_point_.x();
  last_drag_point_ = point;
  if (delta < 0)
    observer_->DidClickLineUp();
  else if (delta > 0)
    observer_->DidClickLineDown();
  return true;
}

bool ScrollBar::HandleMousePressed(const MouseEvent& event) {
  if (event.button != MouseButton::Left)
    return false;
  const auto& point = FloatPoint(event.client_x, event.client_y);
  const auto part = HitTestPoint(point);
  if (part == ScrollBarPart::None)
    return false;
  if (IsDisabled(part))
    return true;
  static_cast<ViewEventTarget*>(window_)->SetCapture();
  active_part_ = part;
  switch (part) {
    case ScrollBarPart::BackwardButton:
      layout_->SetState(ScrollBarPart::BackwardButton, ScrollBarState::Pressed);
      observer_->DidClickLineUp();
      break;
    case ScrollBarPart::BackwardTrack:
      layout_->SetState(ScrollBarPart::Thumb, ScrollBarState::Active);
      observer_->DidClickPageUp();
      break;
    case ScrollBarPart::ForwardButton:
      layout_->SetState(ScrollBarPart::ForwardButton, ScrollBarState::Pressed);
      observer_->DidClickLineDown();
      break;
    case ScrollBarPart::ForwardTrack:
      layout_->SetState(ScrollBarPart::Thumb, ScrollBarState::Active);
      observer_->DidClickPageDown();
      break;
    case ScrollBarPart::Thumb:
      layout_->SetState(ScrollBarPart::Thumb, ScrollBarState::Pressed);
      last_drag_point_ = point;
      break;
  }
  return true;
}

bool ScrollBar::HandleMouseReleased(const MouseEvent& event) {
  if (event.button != MouseButton::Left)
    return false;
  if (active_part_ == ScrollBarPart::None)
    return false;
  static_cast<ViewEventTarget*>(window_)->ReleaseCapture();
  layout_->SetState(active_part_, ScrollBarState::Normal);
  layout_->SetState(ScrollBarPart::Thumb, ScrollBarState::Normal);
  active_part_ = ScrollBarPart::None;
  return true;
}

ScrollBarPart ScrollBar::HitTestPoint(const FloatPoint& point) const {
  return layout_->HitTestPoint(ToPointF(point));
}

bool ScrollBar::IsDisabled(ScrollBarPart part) const {
  return layout_->StateOf(part) == ScrollBarState::Disabled;
}

std::unique_ptr<DisplayItemList> ScrollBar::Paint() const {
  return layout_->Paint();
}

void ScrollBar::SetBounds(const FloatRect& new_bounds) {
  return layout_->SetBounds(ToRectF(new_bounds));
}

void ScrollBar::SetData(const ScrollBarData& new_data) {
  if (data_ == new_data)
    return;
  data_ = new_data;
  SetDisabled(data_.track() == data_.thumb());
  return layout_->SetData(new_data);
}

void ScrollBar::SetDisabled(bool new_disabled) {
  if (disabled_ == new_disabled)
    return;
  disabled_ = new_disabled;
  const auto state =
      disabled_ ? ScrollBarState::Disabled : ScrollBarState::Normal;
  layout_->SetState(ScrollBarPart::BackwardButton, state);
  layout_->SetState(ScrollBarPart::BackwardTrack, state);
  layout_->SetState(ScrollBarPart::ForwardButton, state);
  layout_->SetState(ScrollBarPart::ForwardTrack, state);
  layout_->SetState(ScrollBarPart::Thumb, state);
}

void ScrollBar::UpdateHoveredPart(ScrollBarPart part) {
  if (hovered_part_ == part)
    return;
  if (hovered_part_ != ScrollBarPart::None) {
    layout_->SetState(hovered_part_, IsDisabled(hovered_part_)
                                         ? ScrollBarState::Disabled
                                         : ScrollBarState::Normal);
  }
  hovered_part_ = part;
  if (hovered_part_ == ScrollBarPart::None)
    return;
  if (IsDisabled(hovered_part_)) {
    hovered_part_ = ScrollBarPart::None;
    return;
  }
  layout_->SetState(hovered_part_, ScrollBarState::Hovered);
}

}  // namespace dom
