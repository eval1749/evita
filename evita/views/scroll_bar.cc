// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <cmath>

#include "evita/views/scroll_bar.h"

#include "evita/dom/public/scroll_bar_orientation.h"
#include "evita/dom/public/scroll_bar_part.h"
#include "evita/dom/public/scroll_bar_state.h"
#include "evita/gfx/canvas.h"
#include "evita/gfx/color_f.h"
#include "evita/layout/scroll_bar.h"
#include "evita/ui/controls/scroll_bar_observer.h"
#include "evita/ui/events/event.h"
#include "evita/ui/widget.h"
#include "evita/visuals/display/display_item_list_processor.h"
#include "evita/visuals/display/public/display_item_list.h"

namespace views {

using DisplayItemListProcessor = visuals::DisplayItemListProcessor;

//////////////////////////////////////////////////////////////////////
//
// ScrollBar
//
ScrollBar::ScrollBar(ScrollBarOrientation orientation,
                     ui::Widget* window,
                     ui::ScrollBarObserver* observer)
    : active_part_(ScrollBarPart::None),
      hovered_part_(ScrollBarPart::None),
      layout_(new layout::ScrollBar(orientation)),
      observer_(observer),
      window_(window) {}

ScrollBar::~ScrollBar() {}

bool ScrollBar::HandleMouseMoved(const ui::MouseEvent& event) {
  const auto& point = gfx::PointF(event.location());
  const auto part = HitTestPoint(point);
  if (active_part_ != ScrollBarPart::Thumb) {
    UpdateHoveredPart(part);
    return part != ScrollBarPart::None;
  }
  const auto delta = layout_->IsVertical() ? point.y - last_drag_point_.y
                                           : point.x - last_drag_point_.x;
  last_drag_point_ = point;
  if (delta < 0)
    observer_->DidClickLineUp();
  else if (delta > 0)
    observer_->DidClickLineDown();
  return true;
  return true;
}

bool ScrollBar::HandleMousePressed(const ui::MouseEvent& event) {
  if (event.button() != ui::MouseButton::Left)
    return false;
  const auto& point = gfx::PointF(event.location());
  const auto part = HitTestPoint(point);
  if (part == ScrollBarPart::None)
    return false;
  if (event.click_count() != 0)
    return true;
  window_->SetCapture();
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

bool ScrollBar::HandleMouseReleased(const ui::MouseEvent& event) {
  if (event.button() != ui::MouseButton::Left)
    return false;
  if (active_part_ == ScrollBarPart::None)
    return false;
  window_->ReleaseCapture();
  layout_->SetState(active_part_, ScrollBarState::Normal);
  layout_->SetState(ScrollBarPart::Thumb, ScrollBarState::Normal);
  active_part_ = ScrollBarPart::None;
  return true;
}

ScrollBarPart ScrollBar::HitTestPoint(const gfx::PointF& point) const {
  return layout_->HitTestPoint(point);
}

void ScrollBar::Paint(gfx::Canvas* canvas) const {
  gfx::Canvas::AxisAlignedClipScope clip_scope(canvas, layout_->bounds());
  canvas->Clear(gfx::ColorF(1, 1, 1));
  DisplayItemListProcessor processor;
  processor.Paint(canvas, std::move(layout_->Paint()));
}

void ScrollBar::SetBounds(const gfx::RectF& new_bounds) {
  return layout_->SetBounds(new_bounds);
}

void ScrollBar::SetData(const ScrollBarData& new_data) {
  if (data_ == new_data)
    return;
  data_ = new_data;
  return layout_->SetData(new_data);
}

void ScrollBar::SetDisabled(bool new_disabled) {
  if (disabled_ == new_disabled)
    return;
  disabled_ = new_disabled;
  layout_->SetState(ScrollBarPart::BackwardButton, ScrollBarState::Disabled);
  layout_->SetState(ScrollBarPart::BackwardTrack, ScrollBarState::Disabled);
  layout_->SetState(ScrollBarPart::ForwardButton, ScrollBarState::Disabled);
  layout_->SetState(ScrollBarPart::ForwardTrack, ScrollBarState::Disabled);
  layout_->SetState(ScrollBarPart::Thumb, ScrollBarState::Disabled);
}

void ScrollBar::UpdateHoveredPart(ScrollBarPart part) {
  if (hovered_part_ == part)
    return;
  if (hovered_part_ != ScrollBarPart::None)
    layout_->SetState(hovered_part_, ScrollBarState::Normal);
  hovered_part_ = part;
  if (hovered_part_ != ScrollBarPart::None)
    layout_->SetState(hovered_part_, ScrollBarState::Hovered);
}

}  // namespace views
