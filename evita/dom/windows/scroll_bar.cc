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
#include "evita/text/layout/scroll_bar.h"
#include "evita/ui/controls/scroll_bar_observer.h"
#include "evita/visuals/display/public/display_item_list.h"

namespace dom {

using DisplayItemList = visuals::DisplayItemList;
using FloatPoint = gfx::FloatPoint;
using FloatRect = gfx::FloatRect;
using FloatSize = gfx::FloatSize;
using MouseButton = domapi::MouseButton;
using ScrollBarPart = domapi::ScrollBarPart;

namespace {

gfx::FloatPoint ToFloatPoint(const gfx::PointF& point) {
  return gfx::FloatPoint(point.x, point.y);
}

gfx::FloatSize ToFloatSize(const gfx::SizeF& size) {
  return gfx::FloatSize(size.width, size.height);
}

gfx::FloatRect ToFloatRect(const gfx::RectF& rect) {
  return gfx::FloatRect(ToFloatPoint(rect.origin()), ToFloatSize(rect.size()));
}

gfx::PointF ToPointF(const gfx::FloatPoint& point) {
  return gfx::PointF(point.x(), point.y());
}

gfx::SizeF ToSizeF(const gfx::FloatSize& size) {
  return gfx::SizeF(size.width(), size.height());
}

gfx::RectF ToRectF(const gfx::FloatRect& rect) {
  return gfx::RectF(ToPointF(rect.origin()), ToSizeF(rect.size()));
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// ScrollBarOwner
//
ScrollBarOwner::ScrollBarOwner() {}
ScrollBarOwner::~ScrollBarOwner() {}

//////////////////////////////////////////////////////////////////////
//
// ScrollBar
//
ScrollBar::ScrollBar(ScrollBarOrientation orientation,
                     ScrollBarOwner* owner,
                     ui::ScrollBarObserver* observer)
    : active_part_(ScrollBarPart::None),
      hovered_part_(ScrollBarPart::None),
      layout_(new layout::ScrollBar(orientation)),
      observer_(observer),
      owner_(owner),
      repeat_controller_(
          base::Bind(&ScrollBar::DidFireRepeatTimer, base::Unretained(this))) {
  SetDisabled(true);
}

ScrollBar::~ScrollBar() {}

gfx::FloatRect ScrollBar::bounds() const {
  return ToFloatRect(layout_->bounds());
}

void ScrollBar::DidFireRepeatTimer() {
  switch (active_part_) {
    case ScrollBarPart::BackwardButton:
      observer_->DidClickLineUp();
      break;
    case ScrollBarPart::BackwardTrack:
      observer_->DidClickPageUp();
      break;
    case ScrollBarPart::ForwardButton:
      observer_->DidClickLineDown();
      break;
    case ScrollBarPart::ForwardTrack:
      observer_->DidClickPageDown();
      break;
  }
}

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
  const auto& point = gfx::FloatPoint(event.client_x, event.client_y);
  const auto part = HitTestPoint(point);
  if (active_part_ != ScrollBarPart::Thumb)
    return UpdateHoveredPart(part);
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
  const auto& point = gfx::FloatPoint(event.client_x, event.client_y);
  const auto part = HitTestPoint(point);
  if (part == ScrollBarPart::None)
    return false;
  if (IsDisabled(part))
    return true;
  owner_->DidPressScrollBar();
  active_part_ = part;
  switch (part) {
    case ScrollBarPart::BackwardButton:
      layout_->SetState(ScrollBarPart::BackwardButton, ScrollBarState::Pressed);
      repeat_controller_.Start();
      observer_->DidClickLineUp();
      break;
    case ScrollBarPart::BackwardTrack:
      layout_->SetState(ScrollBarPart::Thumb, ScrollBarState::Active);
      repeat_controller_.Start();
      observer_->DidClickPageUp();
      break;
    case ScrollBarPart::ForwardButton:
      layout_->SetState(ScrollBarPart::ForwardButton, ScrollBarState::Pressed);
      repeat_controller_.Start();
      observer_->DidClickLineDown();
      break;
    case ScrollBarPart::ForwardTrack:
      layout_->SetState(ScrollBarPart::Thumb, ScrollBarState::Active);
      repeat_controller_.Start();
      observer_->DidClickPageDown();
      break;
    case ScrollBarPart::Thumb:
      layout_->SetState(ScrollBarPart::Thumb, ScrollBarState::Pressed);
      repeat_controller_.Start();
      last_drag_point_ = point;
      break;
  }
  owner_->DidChangeScrollBar();
  return true;
}

bool ScrollBar::HandleMouseReleased(const MouseEvent& event) {
  if (event.button != MouseButton::Left)
    return false;
  if (active_part_ == ScrollBarPart::None)
    return false;
  repeat_controller_.Stop();
  owner_->DidReleaseScrollBar();
  layout_->SetState(active_part_, ScrollBarState::Normal);
  layout_->SetState(ScrollBarPart::Thumb, ScrollBarState::Normal);
  active_part_ = ScrollBarPart::None;
  owner_->DidChangeScrollBar();
  return true;
}

ScrollBarPart ScrollBar::HitTestPoint(const gfx::FloatPoint& point) const {
  return layout_->HitTestPoint(ToPointF(point));
}

bool ScrollBar::IsDisabled(ScrollBarPart part) const {
  return layout_->StateOf(part) == ScrollBarState::Disabled;
}

std::unique_ptr<DisplayItemList> ScrollBar::Paint() const {
  return layout_->Paint();
}

void ScrollBar::SetBounds(const gfx::FloatRect& new_bounds) {
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
  owner_->DidChangeScrollBar();
}

bool ScrollBar::UpdateHoveredPart(ScrollBarPart part) {
  if (hovered_part_ == part)
    return false;
  owner_->DidChangeScrollBar();
  if (hovered_part_ != ScrollBarPart::None) {
    layout_->SetState(hovered_part_, IsDisabled(hovered_part_)
                                         ? ScrollBarState::Disabled
                                         : ScrollBarState::Normal);
  }
  hovered_part_ = part;
  if (hovered_part_ == ScrollBarPart::None)
    return true;
  if (IsDisabled(hovered_part_)) {
    hovered_part_ = ScrollBarPart::None;
    return true;
  }
  layout_->SetState(hovered_part_, ScrollBarState::Hovered);
  return true;
}

}  // namespace dom
