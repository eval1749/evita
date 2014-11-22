// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/gfx/bitmap.h"
#include "evita/gfx/canvas.h"
#include "evita/gfx/text_layout.h"
#include "evita/views/icon_cache.h"
#include "evita/views/tabs/tab.h"
#include "evita/views/tab_content.h"

namespace views {

const auto kLabelHeight = 16.0f;
const auto kMaxTabWidth = 200.0f;
const auto kMinTabWidth = 140.0f;

//////////////////////////////////////////////////////////////////////
//
// Tab::HitTestResult
//
Tab::HitTestResult::HitTestResult(Tab* tab, Part part)
    : part_(part), tab_(tab) {
}

Tab::HitTestResult::HitTestResult(const HitTestResult& other)
    : HitTestResult(other.tab_, other.part_) {
}

Tab::HitTestResult::HitTestResult() : HitTestResult(nullptr, Part::None) {
}

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
Tab::Tab(ViewDelegate* view_delegate, TabContent* tab_content,
         gfx::TextFormat* text_format)
    : close_mark_state_(State::Normal),
      dirty_visual_(true),
      dirty_layout_(true),
      image_index_(-1),
      state_(State::Normal),
      tab_data_state_(domapi::TabData::State::Normal),
      tab_content_(tab_content),
      tab_index_(0),
      text_format_(text_format),
      view_delegate_(view_delegate) {
  auto const tab_data = tab_content->GetTabData();
  if (!tab_data) {
    label_text_ = L"?";
    return;
  }
  SetTabData(*tab_data);
}

gfx::ColorF Tab::ComputeBackgroundColor() const {
  switch (state_) {
  case State::Hovered:
    return gfx::ColorF(1.0f, 1.0f, 1.0f, 0.8f);
  case State::Normal:
    return gfx::ColorF(1.0f, 1.0f, 1.0f, 0.5f);
  case State::Selected:
    return gfx::ColorF(1.0f, 1.0f, 1.0f, 1.0f);
  }
  NOTREACHED();
  return gfx::ColorF(1, 0, 0, 1);
}

void Tab::Draw(gfx::Canvas* canvas, const gfx::RectF& content_bounds) {
  UpdateLayout();
  if (!dirty_visual_)
    return;
  dirty_visual_ = false;
  auto const bounds = gfx::RectF(bounds_.origin(),
                                 bounds_.size() + gfx::SizeF(1, 0));
  auto const dirty_bounds = content_bounds.Intersect(bounds);
  if (dirty_bounds.empty())
    return;
  canvas->AddDirtyRect(dirty_bounds);
  gfx::Canvas::AxisAlignedClipScope clip_scope(canvas, dirty_bounds);
  canvas->Clear(ComputeBackgroundColor());
  {
    gfx::Brush strokeBrush(canvas, gfx::ColorF(0, 0, 0, 0.7f));
    canvas->DrawRectangle(strokeBrush, bounds);
  }
  DrawIcon(canvas);
  DrawLabel(canvas);
  DrawCloseMark(canvas);
  DrawTabDataState(canvas);
}

void Tab::DrawCloseMark(gfx::Canvas* canvas) const {
  if (state_ == State::Normal)
    return;
  auto const color = close_mark_state_ == State::Hovered ?
      gfx::ColorF(1, 0, 0, 0.5f) : gfx::ColorF(0, 0, 0, 0.5f);
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
    tab_data_state_ == domapi::TabData::State::Modified ?
        gfx::ColorF(219.0f / 255, 74.0f / 255, 56.0f / 255) :
        gfx::ColorF(56.0f / 255, 219.0f / 255, 74.0f / 255);
  auto const marker_height = 4.0f;
  auto const marker_width = 4.0f;
  DCHECK_GT(bounds_.width(), marker_width);
  canvas->FillRectangle(
      gfx::Brush(canvas, marker_color),
      gfx::RectF(gfx::PointF(bounds_.right - marker_width, bounds_.top),
                 gfx::SizeF(marker_width, marker_height)));
}

int Tab::GetPreferredWidth() const {
  DCHECK(text_format_);
  auto const label_width = std::min(
      std::max(text_format_->GetWidth(label_text_), kMinTabWidth),
      kMaxTabWidth);
  auto const icon_width = 16.0f;
  auto const padding = 6.0f;
  return static_cast<int>(::ceil(
    padding + icon_width + 4 + label_width + 2 + 9 + padding));
}

Tab::State Tab::GetState(Part part) const {
  return part == Part::Label ? state_ : close_mark_state_;
}

Tab::HitTestResult Tab::HitTest(const gfx::PointF& point) {
  UpdateLayout();
  if (!bounds_.Contains(point))
    return HitTestResult();
  if (state_ != State::Normal && close_mark_bounds_.Contains(point))
    return HitTestResult(const_cast<Tab*>(this), Part::CloseMark);
  return HitTestResult(const_cast<Tab*>(this), Part::Label);
}

void Tab::MarkDirty() {
  dirty_visual_ = true;
  view_delegate_->RequestAnimationFrame();
}

void Tab::SetBounds(const gfx::RectF& new_bounds) {
  if (bounds_ == new_bounds)
    return;
  bounds_ = new_bounds;
  MarkDirty();
  dirty_layout_ = true;
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

void Tab::UpdateLayout() {
  DCHECK(!bounds_.empty());
  DCHECK(text_format_);
  if (!dirty_layout_)
    return;
  dirty_layout_ = false;
  dirty_visual_ = true;
  view_delegate_->RequestAnimationFrame();
  auto const bounds = bounds_ - gfx::SizeF(6, 6);
  close_mark_bounds_ = gfx::RectF(bounds.top_right() + gfx::SizeF(-9, 5),
                                  gfx::SizeF(8, 8));
  icon_bounds_ = gfx::RectF(bounds.origin(), gfx::SizeF(16, 16));
  label_bounds_ = gfx::RectF(icon_bounds_.top_right() + gfx::SizeF(4, 0),
                             gfx::PointF(close_mark_bounds_.left - 2,
                                         icon_bounds_.bottom));
  text_layout_ = text_format_->CreateLayout(label_text_, label_bounds_.size());
}

// ui::Tooltip::ToolDelegate
base::string16 Tab::GetTooltipText() {
  return view_delegate_->GetTooltipTextForTab(this);
}

}  // namespace views
