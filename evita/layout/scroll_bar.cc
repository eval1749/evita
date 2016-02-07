// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>

#include "evita/layout/scroll_bar.h"

#include "evita/dom/public/scroll_bar_orientation.h"
#include "evita/dom/public/scroll_bar_part.h"
#include "evita/dom/public/scroll_bar_state.h"
#include "evita/paint/scroll_bar_painter.h"
#include "evita/visuals/display/display_item_list_builder.h"
#include "evita/visuals/display/public/display_item_list.h"

namespace layout {

using DisplayItemList = visuals::DisplayItemList;
using DisplayItemListBuilder = visuals::DisplayItemListBuilder;
using FloatPoint = visuals::FloatPoint;
using FloatSize = visuals::FloatSize;
using FloatRect = visuals::FloatRect;
using Part = ScrollBar::Part;
using ScrollBarPainter = paint::ScrollBarPainter;

namespace {

float ComputeHorizontalThumbLeft(const gfx::RectF& bounds,
                                 const ScrollBarData& data) {
  const auto button_size = bounds.height();
  const auto track_size = bounds.width() - button_size * 2;
  const auto ratio = data.thumb().lower() / data.track().length();
  return ratio * track_size + bounds.left + button_size;
}

float ComputeHorizontalThumbRight(const gfx::RectF& bounds,
                                  const ScrollBarData data) {
  if (data.thumb().empty())
    return ComputeHorizontalThumbLeft(bounds, data);
  const auto button_size = bounds.height();
  const auto track_size = bounds.width() - button_size * 2;
  const auto ratio = data.thumb().upper() / data.track().length();
  return std::max(ratio * track_size, button_size) + bounds.left + button_size;
}

float ComputeVerticalThumbTop(const gfx::RectF& bounds,
                              const ScrollBarData& data) {
  const auto button_size = bounds.width();
  const auto track_size = bounds.height() - button_size * 2;
  const auto ratio = data.thumb().lower() / data.track().length();
  return ratio * track_size + bounds.top + button_size;
}

float ComputeVerticalThumbBottom(const gfx::RectF& bounds,
                                 const ScrollBarData data) {
  if (data.thumb().empty())
    return ComputeVerticalThumbTop(bounds, data);
  const auto button_size = bounds.width();
  const auto track_size = bounds.height() - button_size * 2;
  const auto ratio = data.thumb().upper() / data.track().length();
  return std::max(ratio * track_size, button_size) + bounds.top + button_size;
}

FloatPoint ToFloatPoint(const gfx::PointF& point) {
  return FloatPoint(point.x, point.y);
}

FloatSize ToFloatSize(const gfx::SizeF& size) {
  return FloatSize(size.width, size.height);
}

FloatRect ToFloatRect(const gfx::RectF& rect) {
  return FloatRect(ToFloatPoint(rect.origin()), ToFloatSize(rect.size()));
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// Part
//
class ScrollBar::Part {
 public:
  virtual ~Part() = default;

  const gfx::RectF& bounds() const { return bounds_; }
  virtual ScrollBarPart part() const = 0;

  virtual void DidChangeBounds(const gfx::RectF& new_bounds,
                               const gfx::RectF& old_bounds);
  virtual void DidChangeData(const ScrollBarData& new_data,
                             const ScrollBarData& old_data);
  void Paint(DisplayItemListBuilder* builder);
  void SetBounds(const gfx::RectF& new_bounds);
  void SetState(ScrollBarState new_state);
  void UpdateLayoutIfNeeded(const gfx::RectF& bounds,
                            const ScrollBarData& data);

 protected:
  Part() = default;

  ScrollBarState state() const { return state_; }

  void MarkLayoutDirty();
  virtual void PaintContent(DisplayItemListBuilder* builder) const = 0;
  virtual void UpdateLayout(const gfx::RectF& bounds,
                            const ScrollBarData& data) = 0;

 private:
  gfx::RectF bounds_;
  bool layout_dirty_ = false;
  bool paint_dirty_ = false;
  ScrollBarState state_ = ScrollBarState::Normal;

  DISALLOW_COPY_AND_ASSIGN(Part);
};

void Part::DidChangeBounds(const gfx::RectF& new_bounds,
                           const gfx::RectF& old_bounds) {
  MarkLayoutDirty();
}

void Part::DidChangeData(const ScrollBarData& new_data,
                         const ScrollBarData& old_data) {
  MarkLayoutDirty();
}

void Part::MarkLayoutDirty() {
  layout_dirty_ = true;
  paint_dirty_ = true;
}

void Part::Paint(DisplayItemListBuilder* builder) {
  if (bounds_.empty()) {
    paint_dirty_ = false;
    return;
  }
  PaintContent(builder);
  if (!paint_dirty_)
    return;
  paint_dirty_ = false;
  builder->AddRect(ToFloatRect(bounds_));
}

void Part::SetBounds(const gfx::RectF& new_bounds) {
  DCHECK(layout_dirty_);
  if (bounds_ == new_bounds)
    return;
  bounds_ = new_bounds;
}

void Part::SetState(ScrollBarState new_state) {
  if (state_ == new_state)
    return;
  state_ = new_state;
  paint_dirty_ = true;
}

void Part::UpdateLayoutIfNeeded(const gfx::RectF& bounds,
                                const ScrollBarData& data) {
  if (!layout_dirty_)
    return;
  UpdateLayout(bounds, data);
  layout_dirty_ = false;
}

//////////////////////////////////////////////////////////////////////
//
// BottomButton
//
class BottomButton final : public Part {
 public:
  BottomButton() = default;
  ~BottomButton() final = default;

 private:
  // Part
  ScrollBarPart part() const final { return ScrollBarPart::ForwardButton; }

  void DidChangeBounds(const gfx::RectF& new_bounds,
                       const gfx::RectF& old_bounds) final;
  void DidChangeData(const ScrollBarData& new_data,
                     const ScrollBarData& old_data) {}
  void PaintContent(DisplayItemListBuilder* builder) const final;
  void UpdateLayout(const gfx::RectF& bounds, const ScrollBarData& data) final;

  DISALLOW_COPY_AND_ASSIGN(BottomButton);
};

void BottomButton::DidChangeBounds(const gfx::RectF& new_bounds,
                                   const gfx::RectF& old_bounds) {
  if (new_bounds.width() == old_bounds.width())
    return;
  MarkLayoutDirty();
}

void BottomButton::PaintContent(DisplayItemListBuilder* builder) const {
  ScrollBarPainter().PaintBottomButton(builder, ToFloatRect(bounds()), state());
}

void BottomButton::UpdateLayout(const gfx::RectF& bounds,
                                const ScrollBarData& data) {
  const auto size = bounds.width();
  SetBounds(gfx::RectF(bounds.bottom_left() - gfx::SizeF(0.0f, size),
                       gfx::SizeF(size, size)));
}

//////////////////////////////////////////////////////////////////////
//
// BottomTrack
//
class BottomTrack final : public Part {
 public:
  BottomTrack() = default;
  ~BottomTrack() final = default;

 private:
  // Part
  ScrollBarPart part() const final { return ScrollBarPart::ForwardTrack; }

  void PaintContent(DisplayItemListBuilder* builder) const final;
  void UpdateLayout(const gfx::RectF& bounds, const ScrollBarData& data) final;

  DISALLOW_COPY_AND_ASSIGN(BottomTrack);
};

void BottomTrack::PaintContent(DisplayItemListBuilder* builder) const {
  ScrollBarPainter().PaintBottomTrack(builder, ToFloatRect(bounds()), state());
}

void BottomTrack::UpdateLayout(const gfx::RectF& bounds,
                               const ScrollBarData& data) {
  const auto thumb_bottom = ComputeVerticalThumbBottom(bounds, data);
  SetBounds(
      gfx::RectF(gfx::PointF(bounds.left, thumb_bottom),
                 bounds.bottom_right() - gfx::SizeF(0.0f, bounds.width())));
}

//////////////////////////////////////////////////////////////////////
//
// HorizontalThumb
//
class HorizontalThumb final : public Part {
 public:
  HorizontalThumb() = default;
  ~HorizontalThumb() final = default;

 private:
  // Part
  ScrollBarPart part() const final { return ScrollBarPart::Thumb; }

  void PaintContent(DisplayItemListBuilder* builder) const final;
  void UpdateLayout(const gfx::RectF& bounds, const ScrollBarData& data) final;

  DISALLOW_COPY_AND_ASSIGN(HorizontalThumb);
};

void HorizontalThumb::PaintContent(DisplayItemListBuilder* builder) const {
  ScrollBarPainter().PaintHorizontalThumb(builder, ToFloatRect(bounds()),
                                          state());
}

void HorizontalThumb::UpdateLayout(const gfx::RectF& bounds,
                                   const ScrollBarData& data) {
  SetBounds(gfx::RectF(
      gfx::PointF(ComputeHorizontalThumbLeft(bounds, data), bounds.top),
      gfx::PointF(ComputeHorizontalThumbRight(bounds, data), bounds.bottom)));
}

//////////////////////////////////////////////////////////////////////
//
// LeftButton
//
class LeftButton final : public Part {
 public:
  LeftButton() = default;
  ~LeftButton() final = default;

 private:
  // Part
  ScrollBarPart part() const final { return ScrollBarPart::BackwardButton; }

  void DidChangeBounds(const gfx::RectF& new_bounds,
                       const gfx::RectF& old_bounds) final;
  void DidChangeData(const ScrollBarData& new_data,
                     const ScrollBarData& old_data) final {}
  void PaintContent(DisplayItemListBuilder* builder) const final;
  void UpdateLayout(const gfx::RectF& bounds, const ScrollBarData& data) final;

  DISALLOW_COPY_AND_ASSIGN(LeftButton);
};

void LeftButton::DidChangeBounds(const gfx::RectF& new_bounds,
                                 const gfx::RectF& old_bounds) {
  if (new_bounds.height() == old_bounds.height())
    return;
  MarkLayoutDirty();
}

void LeftButton::PaintContent(DisplayItemListBuilder* builder) const {
  ScrollBarPainter().PaintLeftButton(builder, ToFloatRect(bounds()), state());
}

void LeftButton::UpdateLayout(const gfx::RectF& bounds,
                              const ScrollBarData& data) {
  const auto size = bounds.height();
  SetBounds(gfx::RectF(bounds.origin(), gfx::SizeF(size, size)));
}

//////////////////////////////////////////////////////////////////////
//
// LeftTrack
//
class LeftTrack final : public Part {
 public:
  LeftTrack() = default;
  ~LeftTrack() final = default;

 private:
  // Part
  ScrollBarPart part() const final { return ScrollBarPart::BackwardTrack; }

  void PaintContent(DisplayItemListBuilder* builder) const final;
  void UpdateLayout(const gfx::RectF& bounds, const ScrollBarData& data) final;

  DISALLOW_COPY_AND_ASSIGN(LeftTrack);
};

void LeftTrack::PaintContent(DisplayItemListBuilder* builder) const {
  ScrollBarPainter().PaintLeftTrack(builder, ToFloatRect(bounds()), state());
}

void LeftTrack::UpdateLayout(const gfx::RectF& bounds,
                             const ScrollBarData& data) {
  SetBounds(gfx::RectF(
      bounds.origin() + gfx::SizeF(bounds.height(), 0.0f),
      gfx::PointF(ComputeHorizontalThumbLeft(bounds, data), bounds.bottom)));
}

//////////////////////////////////////////////////////////////////////
//
// RightButton
//
class RightButton final : public Part {
 public:
  RightButton() = default;
  ~RightButton() final = default;

 private:
  // Part
  ScrollBarPart part() const final { return ScrollBarPart::ForwardButton; }

  void DidChangeBounds(const gfx::RectF& new_bounds,
                       const gfx::RectF& old_bounds) final;
  void DidChangeData(const ScrollBarData& new_data,
                     const ScrollBarData& old_data) {}
  void PaintContent(DisplayItemListBuilder* builder) const final;
  void UpdateLayout(const gfx::RectF& bounds, const ScrollBarData& data) final;

  DISALLOW_COPY_AND_ASSIGN(RightButton);
};

void RightButton::DidChangeBounds(const gfx::RectF& new_bounds,
                                  const gfx::RectF& old_bounds) {
  if (new_bounds.height() == old_bounds.height())
    return;
  MarkLayoutDirty();
}

void RightButton::PaintContent(DisplayItemListBuilder* builder) const {
  ScrollBarPainter().PaintRightButton(builder, ToFloatRect(bounds()), state());
}

void RightButton::UpdateLayout(const gfx::RectF& bounds,
                               const ScrollBarData& data) {
  const auto size = bounds.height();
  SetBounds(gfx::RectF(bounds.origin() - gfx::SizeF(size, 0.0f),
                       gfx::SizeF(size, size)));
}

//////////////////////////////////////////////////////////////////////
//
// RightTrack
//
class RightTrack final : public Part {
 public:
  RightTrack() = default;
  ~RightTrack() final = default;

 private:
  // Part
  ScrollBarPart part() const final { return ScrollBarPart::ForwardTrack; }

  void PaintContent(DisplayItemListBuilder* builder) const final;
  void UpdateLayout(const gfx::RectF& bounds, const ScrollBarData& data) final;

  DISALLOW_COPY_AND_ASSIGN(RightTrack);
};

void RightTrack::PaintContent(DisplayItemListBuilder* builder) const {
  ScrollBarPainter().PaintRightTrack(builder, ToFloatRect(bounds()), state());
}

void RightTrack::UpdateLayout(const gfx::RectF& bounds,
                              const ScrollBarData& data) {
  SetBounds(gfx::RectF(
      gfx::PointF(ComputeHorizontalThumbRight(bounds, data), bounds.top),
      bounds.bottom_right() - gfx::SizeF(bounds.height(), 0.0f)));
}

//////////////////////////////////////////////////////////////////////
//
// TopButton
//
class TopButton final : public Part {
 public:
  TopButton() = default;
  ~TopButton() final = default;

 private:
  // Part
  ScrollBarPart part() const final { return ScrollBarPart::BackwardButton; }

  void DidChangeBounds(const gfx::RectF& new_bounds,
                       const gfx::RectF& old_bounds) final;
  void DidChangeData(const ScrollBarData& new_data,
                     const ScrollBarData& old_data) final {}
  void PaintContent(DisplayItemListBuilder* builder) const final;
  void UpdateLayout(const gfx::RectF& bounds, const ScrollBarData& data) final;

  DISALLOW_COPY_AND_ASSIGN(TopButton);
};

void TopButton::DidChangeBounds(const gfx::RectF& new_bounds,
                                const gfx::RectF& old_bounds) {
  if (new_bounds.width() == old_bounds.width())
    return;
  MarkLayoutDirty();
}

void TopButton::PaintContent(DisplayItemListBuilder* builder) const {
  ScrollBarPainter().PaintTopButton(builder, ToFloatRect(bounds()), state());
}

void TopButton::UpdateLayout(const gfx::RectF& bounds,
                             const ScrollBarData& data) {
  const auto size = bounds.width();
  SetBounds(gfx::RectF(bounds.origin(), gfx::SizeF(size, size)));
}

//////////////////////////////////////////////////////////////////////
//
// TopTrack
//
class TopTrack final : public Part {
 public:
  TopTrack() = default;
  ~TopTrack() final = default;

 private:
  // Part
  ScrollBarPart part() const final { return ScrollBarPart::BackwardTrack; }

  void PaintContent(DisplayItemListBuilder* builder) const final;
  void UpdateLayout(const gfx::RectF& bounds, const ScrollBarData& data) final;

  DISALLOW_COPY_AND_ASSIGN(TopTrack);
};

void TopTrack::PaintContent(DisplayItemListBuilder* builder) const {
  ScrollBarPainter().PaintTopTrack(builder, ToFloatRect(bounds()), state());
}

void TopTrack::UpdateLayout(const gfx::RectF& bounds,
                            const ScrollBarData& data) {
  const auto thumb_top = ComputeVerticalThumbTop(bounds, data);
  SetBounds(gfx::RectF(bounds.origin() + gfx::SizeF(0.0f, bounds.width()),
                       gfx::PointF(bounds.right, thumb_top)));
}

//////////////////////////////////////////////////////////////////////
//
// VerticalThumb
//
class VerticalThumb final : public Part {
 public:
  VerticalThumb() = default;
  ~VerticalThumb() final = default;

 private:
  // Part
  ScrollBarPart part() const final { return ScrollBarPart::Thumb; }

  void PaintContent(DisplayItemListBuilder* builder) const final;
  void UpdateLayout(const gfx::RectF& bounds, const ScrollBarData& data) final;

  DISALLOW_COPY_AND_ASSIGN(VerticalThumb);
};

void VerticalThumb::PaintContent(DisplayItemListBuilder* builder) const {
  ScrollBarPainter().PaintVerticalThumb(builder, ToFloatRect(bounds()),
                                        state());
}

void VerticalThumb::UpdateLayout(const gfx::RectF& bounds,
                                 const ScrollBarData& data) {
  const auto top = ComputeVerticalThumbTop(bounds, data);
  const auto bottom = ComputeVerticalThumbBottom(bounds, data);
  SetBounds(gfx::RectF(gfx::PointF(bounds.left, top),
                       gfx::PointF(bounds.right, bottom)));
}

namespace {

std::vector<Part*> CreateParts(ScrollBarOrientation orientation) {
  if (orientation == ScrollBarOrientation::Horizontal) {
    return {
        new LeftButton(), new LeftTrack(),   new HorizontalThumb(),
        new RightTrack(), new RightButton(),
    };
  }
  return {
      new TopButton(),   new TopTrack(),     new VerticalThumb(),
      new BottomTrack(), new BottomButton(),
  };
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// ScrollBar
//
ScrollBar::ScrollBar(ScrollBarOrientation orientation)
    : parts_(CreateParts(orientation)), orientation_(orientation) {}

ScrollBar::~ScrollBar() {
  for (const auto& part : parts_)
    delete part;
}

Part* ScrollBar::FindPart(ScrollBarPart part_name) const {
  for (const auto& part : parts_) {
    if (part->part() == part_name)
      return part;
  }
  return nullptr;
}

ScrollBarPart ScrollBar::HitTestPoint(const gfx::PointF& point) const {
  for (const auto& part : parts_) {
    if (part->bounds().Contains(point))
      return part->part();
  }
  return ScrollBarPart::None;
}

bool ScrollBar::IsVertical() const {
  return orientation_ == ScrollBarOrientation::Vertical;
}

std::unique_ptr<DisplayItemList> ScrollBar::Paint() {
  UpdateLayoutIfNeeded();
  DisplayItemListBuilder builder(ToFloatRect(bounds_));
  for (const auto& part : parts_)
    part->Paint(&builder);
  return std::move(builder.Build());
}

void ScrollBar::SetBounds(const gfx::RectF& new_bounds) {
  if (bounds_ == new_bounds)
    return;
  for (const auto& part : parts_)
    part->DidChangeBounds(new_bounds, bounds_);
  bounds_ = new_bounds;
}

void ScrollBar::SetData(const ScrollBarData& new_data) {
  if (data_ == new_data)
    return;
  for (const auto& part : parts_) {
    part->DidChangeData(new_data, data_);
  }
  data_ = new_data;
}

void ScrollBar::SetState(ScrollBarPart part_name, ScrollBarState new_state) {
  const auto part = FindPart(part_name);
  if (!part)
    return;
  part->SetState(new_state);
}

void ScrollBar::UpdateLayoutIfNeeded() {
  for (const auto& part : parts_)
    part->UpdateLayoutIfNeeded(bounds_, data_);
}

}  // namespace layout
