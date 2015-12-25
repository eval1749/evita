// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>

#include "evita/gfx/bitmap.h"
#include "evita/gfx/brush.h"
#include "evita/gfx/canvas.h"
#include "evita/gfx/text_layout.h"
#include "evita/ui/animation/animation_value.h"
#include "evita/ui/events/event.h"
#include "evita/views/icon_cache.h"
#include "evita/views/tabs/tab.h"
#include "evita/views/tabs/tab_content.h"

namespace views {

namespace {

const auto kLabelHeight = 16.0f;
const auto kMaxTabWidth = 200.0f;
const auto kMinTabWidth = 140.0f;

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// TabController
//
TabController::TabController() {}

TabController::~TabController() {}

//////////////////////////////////////////////////////////////////////
//
// Tab::HitTestResult
//
Tab::HitTestResult::HitTestResult(Tab* tab, Part part)
    : part_(part), tab_(tab) {}

Tab::HitTestResult::HitTestResult(const HitTestResult& other)
    : HitTestResult(other.tab_, other.part_) {}

Tab::HitTestResult::HitTestResult() : HitTestResult(nullptr, Part::None) {}

Tab::HitTestResult& Tab::HitTestResult::operator=(const HitTestResult& other) {
  tab_ = other.tab_;
  part_ = other.part_;
  return *this;
}

bool Tab::HitTestResult::operator==(const HitTestResult& other) const {
  return tab_ == other.tab_ && part_ == other.part_;
}

bool Tab::HitTestResult::operator!=(const HitTestResult& other) const {
  return !operator==(other);
}

//////////////////////////////////////////////////////////////////////
//
// Tab
//
Tab::Tab(TabController* tab_controller,
         TabContent* tab_content,
         gfx::TextFormat* text_format)
    : animated_alpha_(ComputeAlpha(State::Normal)),
      close_mark_state_(State::Normal),
      dirty_visual_(true),
      dirty_layout_(true),
      image_index_(-1),
      state_(State::Normal),
      tab_data_state_(domapi::TabData::State::Normal),
      tab_content_(tab_content),
      tab_index_(0),
      text_format_(text_format),
      tab_controller_(tab_controller) {
  auto const tab_data = tab_content->GetTabData();
  if (!tab_data) {
    label_text_ = L"?";
    return;
  }
  SetTabData(*tab_data);
}

Tab::~Tab() {}

float Tab::ComputeAlpha(State state) {
  switch (state) {
    case State::Hovered:
      return 0.8f;
    case State::Normal:
      return 0.5f;
    case State::Selected:
      return 1.0f;
  }
  NOTREACHED();
  return 1.0f;
}

gfx::ColorF Tab::ComputeBackgroundColor() const {
  switch (state_) {
    case State::Hovered:
      return gfx::ColorF(0.9f, 0.9f, 0.9f, animated_alpha_);
    case State::Normal:
      return gfx::ColorF(0.8f, 0.8f, 0.8f, animated_alpha_);
    case State::Selected:
      return gfx::ColorF(1.0f, 1.0f, 1.0f, 1.0f);
  }
  NOTREACHED();
  return gfx::ColorF(1, 0, 0, 1);
}

void Tab::DrawCloseMark(gfx::Canvas* canvas) const {
  if (state_ == State::Normal)
    return;
  auto const color = close_mark_state_ == State::Hovered
                         ? gfx::ColorF(1, 0, 0, 0.5f)
                         : gfx::ColorF(0, 0, 0, 0.5f);
  gfx::Brush brush(canvas, color);
  canvas->DrawLine(brush, close_mark_bounds_.origin(),
                   close_mark_bounds_.bottom_right(), 2.0f);
  canvas->DrawLine(brush, close_mark_bounds_.top_right(),
                   close_mark_bounds_.bottom_left(), 2.0f);
}

void Tab::DrawLabel(gfx::Canvas* canvas) const {
  gfx::Canvas::AxisAlignedClipScope clip_scope(canvas, label_bounds_);
  gfx::Brush text_brush(canvas, gfx::sysColor(COLOR_BTNTEXT));
  (*canvas)->DrawTextLayout(label_bounds_.origin(), *text_layout_, text_brush,
                            D2D1_DRAW_TEXT_OPTIONS_CLIP);
}

void Tab::DrawIcon(gfx::Canvas* canvas) const {
  if (image_index_ < 0)
    return;
  auto const hImageList = IconCache::instance()->image_list();
  // Note: ILD_TRANSPARENT doesn't effect.
  // Note: ILD_DPISCALE makes background black.
  auto const hIcon = ::ImageList_GetIcon(hImageList, image_index_, 0);
  if (!hIcon)
    return;
  gfx::Bitmap bitmap(canvas, hIcon);
  (*canvas)->DrawBitmap(bitmap, icon_bounds_);
  ::DestroyIcon(hIcon);
}

void Tab::DrawTabDataState(gfx::Canvas* canvas) const {
  if (tab_data_state_ == domapi::TabData::State::Normal)
    return;
  auto const marker_color =
      tab_data_state_ == domapi::TabData::State::Modified
          ? gfx::ColorF(219.0f / 255, 74.0f / 255, 56.0f / 255)
          : gfx::ColorF(56.0f / 255, 219.0f / 255, 74.0f / 255);
  auto const marker_height = 4.0f;
  auto const marker_width = 4.0f;
  DCHECK_GT(GetContentsBounds().width(), marker_width);
  canvas->FillRectangle(
      gfx::Brush(canvas, marker_color),
      gfx::RectF(gfx::PointF(GetContentsBounds().right - marker_width,
                             GetContentsBounds().top),
                 gfx::SizeF(marker_width, marker_height)));
}

int Tab::GetPreferredWidth() const {
  DCHECK(text_format_);
  auto const label_width =
      std::min(std::max(text_format_->GetWidth(label_text_), kMinTabWidth),
               kMaxTabWidth);
  auto const icon_width = 16.0f;
  auto const padding = 6.0f;
  return static_cast<int>(
      ::ceil(padding + icon_width + 4 + label_width + 2 + 9 + padding));
}

Tab::State Tab::GetState(Part part) const {
  return part == Part::Label ? state_ : close_mark_state_;
}

Tab::HitTestResult Tab::HitTest(const gfx::PointF& point) {
  UpdateLayout();
  if (!GetContentsBounds().Contains(point))
    return HitTestResult();
  if (state_ != State::Normal && close_mark_bounds_.Contains(point))
    return HitTestResult(const_cast<Tab*>(this), Part::CloseMark);
  return HitTestResult(const_cast<Tab*>(this), Part::Label);
}

void Tab::MarkDirty() {
  dirty_visual_ = true;
  if (!is_realized())
    return;
  SchedulePaint();
}

void Tab::Select() {
  SetState(Tab::Part::Label, Tab::State::Selected);
}

void Tab::SetCloseMarkState(State new_state) {
  if (close_mark_state_ == new_state)
    return;
  close_mark_state_ = new_state;
  MarkDirty();
}

void Tab::SetLabelState(State new_state) {
  if (state_ == new_state)
    return;
  animation_alpha_.reset();
  if (state_ != State::Selected && new_state != State::Selected) {
    tab_controller_->AddTabAnimation(this);
    animation_alpha_.reset(
        new ui::AnimationFloat(base::TimeDelta::FromMilliseconds(16 * 20),
                               ComputeAlpha(state_), ComputeAlpha(new_state)));
  }
  state_ = new_state;
  MarkDirty();
}

void Tab::SetState(Part part, State new_state) {
  switch (new_state) {
    case State::Normal:
      if (part == Part::Label)
        SetLabelState(State::Normal);
      SetCloseMarkState(State::Normal);
      break;
    case State::Hovered:
      if (state_ == State::Normal)
        SetLabelState(State::Hovered);
      if (part == Part::CloseMark)
        SetCloseMarkState(State::Hovered);
      break;
    case State::Selected:
      SetLabelState(State::Selected);
      SetCloseMarkState(State::Normal);
      break;
  }
}

void Tab::SetTabData(const domapi::TabData& tab_data) {
  struct Local {
    static int GetIconIndex(const domapi::TabData& tab_data) {
      if (tab_data.icon != -2)
        return tab_data.icon;
      return IconCache::instance()->GetIconForFileName(tab_data.title);
    }
  };

  auto const new_image_index = Local::GetIconIndex(tab_data);
  if (image_index_ != new_image_index) {
    image_index_ = std::max(new_image_index, 0);
    MarkDirty();
  }

  if (tab_data_state_ != tab_data.state) {
    tab_data_state_ = tab_data.state;
    MarkDirty();
  }

  if (label_text_ != tab_data.title) {
    label_text_ = tab_data.title;
    MarkDirty();
    dirty_layout_ = true;
  }
}

void Tab::SetTextFormat(gfx::TextFormat* text_format) {
  text_format_ = text_format;
  MarkDirty();
  dirty_layout_ = true;
}

void Tab::Unselect() {
  SetState(Tab::Part::Label, Tab::State::Normal);
}

void Tab::UpdateLayout() {
  DCHECK(!GetContentsBounds().empty());
  DCHECK(text_format_);
  if (!dirty_layout_)
    return;
  dirty_layout_ = false;
  MarkDirty();
  auto const bounds = GetContentsBounds() - gfx::SizeF(6, 6);
  close_mark_bounds_ =
      gfx::RectF(bounds.top_right() + gfx::SizeF(-9, 5), gfx::SizeF(8, 8));
  icon_bounds_ = gfx::RectF(bounds.origin(), gfx::SizeF(16, 16));
  label_bounds_ =
      gfx::RectF(icon_bounds_.top_right() + gfx::SizeF(4, 0),
                 gfx::PointF(close_mark_bounds_.left - 2, icon_bounds_.bottom));
  text_layout_ = text_format_->CreateLayout(label_text_, label_bounds_.size());
}

// ui::AnimationGroupMember
void Tab::Animate(base::Time time) {
  if (!animation_alpha_)
    return;
  if (!animation_alpha_->is_started())
    animation_alpha_->Start(time);
  MarkDirty();
  animated_alpha_ = animation_alpha_->Compute(time);
  if (animated_alpha_ == animation_alpha_->end_value()) {
    animation_alpha_.reset();
    return;
  }
  tab_controller_->AddTabAnimation(this);
}

// ui::Tooltip::ToolDelegate
base::string16 Tab::GetTooltipText() {
  auto const tab_data = tab_content_->GetTabData();
  if (!tab_data)
    return base::string16();
  return tab_data->tooltip;
}

// ui::Widget
void Tab::DidChangeBounds() {
  auto const new_bounds = GetContentsBounds();
  if (GetContentsBounds() != new_bounds) {
    GetContentsBounds() = new_bounds;
    dirty_layout_ = true;
  }
  // Since, tab is paint into parent's canvas, we should paint tab event if it
  // moved.
  MarkDirty();
  tab_controller_->DidChangeTabBounds(this);
}

void Tab::OnDraw(gfx::Canvas* canvas) {
  UpdateLayout();
  if (!dirty_visual_)
    return;
  dirty_visual_ = false;
  auto const bounds = GetContentsBounds();
  canvas->AddDirtyRect(bounds);
  gfx::Canvas::AxisAlignedClipScope clip_scope(canvas, bounds);
  canvas->Clear(ComputeBackgroundColor());
  {
    common::ComPtr<ID2D1PathGeometry> geometry;
    canvas->GetD2D1Factory()->CreatePathGeometry(&geometry);
    {
      common::ComPtr<ID2D1GeometrySink> sink;
      geometry->Open(&sink);
      sink->BeginFigure(bounds.origin(), D2D1_FIGURE_BEGIN_HOLLOW);
      sink->AddLine(bounds.top_right());
      sink->AddLine(bounds.bottom_right());
      sink->AddLine(bounds.bottom_left());
      sink->EndFigure(previous_sibling() ? D2D1_FIGURE_END_OPEN
                                         : D2D1_FIGURE_END_CLOSED);
      sink->Close();
    }

    gfx::Brush strokeBrush(canvas, gfx::ColorF(0, 0, 0, 0.7f));
    (*canvas)->DrawGeometry(geometry, strokeBrush);
  }
  DrawIcon(canvas);
  DrawLabel(canvas);
  DrawCloseMark(canvas);
  DrawTabDataState(canvas);
}

void Tab::OnMouseEntered(const ui::MouseEvent& event) {
  if (state_ == State::Normal)
    SetLabelState(State::Hovered);
  auto const result = HitTest(gfx::PointF(event.location()));
  if (result.part() == Part::CloseMark)
    SetCloseMarkState(State::Hovered);
}

void Tab::OnMouseExited(const ui::MouseEvent&) {
  if (is_selected())
    return;
  SetLabelState(State::Normal);
  SetCloseMarkState(State::Normal);
}

void Tab::OnMouseMoved(const ui::MouseEvent& event) {
  if (state_ == State::Normal)
    SetLabelState(State::Hovered);
  auto const result = HitTest(gfx::PointF(event.location()));
  SetCloseMarkState(result.part() == Part::CloseMark ? State::Hovered
                                                     : State::Normal);
}

void Tab::OnMousePressed(const ui::MouseEvent& event) {
  if (!event.is_left_button() || event.click_count())
    return;
  auto const result = HitTest(gfx::PointF(event.location()));
  if (result.part() != Part::Label)
    return;
  if (!is_selected())
    tab_controller_->RequestSelectTab(this);
  tab_controller_->MaybeStartDrag(this, event.location());
}

void Tab::OnMouseReleased(const ui::MouseEvent& event) {
  if (!event.is_left_button() || event.click_count())
    return;
  // TODO(eval1749): stop tab dragging
  auto const result = HitTest(gfx::PointF(event.location()));
  if (result.part() == Part::CloseMark)
    tab_controller_->RequestCloseTab(this);
}

}  // namespace views
