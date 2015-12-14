// Copyright (c) 1996-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>
#include <memory>
#include <utility>
#include <vector>

#include "evita/layout/text_view.h"

#include "base/trace_event/trace_event.h"
#include "evita/dom/lock.h"
#include "evita/editor/dom_lock.h"
#include "evita/paint/view_painter.h"
#include "evita/paint/view_paint_cache.h"
#include "evita/text/buffer.h"
#include "evita/layout/layout_view.h"
#include "evita/layout/layout_block_flow.h"
#include "evita/layout/layout_view_builder.h"

namespace layout {

namespace {
text::Offset GetCaretOffset(const text::Buffer* buffer,
                            const TextSelectionModel& selection,
                            text::Offset caret_offset) {
  if (!selection.disabled())
    return selection.focus_offset();
  auto const max_offset = buffer->GetEnd();
  if (selection.start() == max_offset && selection.end() == max_offset)
    return max_offset;
  return caret_offset.IsValid() ? caret_offset : selection.focus_offset();
}
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// TextView
//
TextView::TextView(text::Buffer* buffer, ui::AnimatableWindow* caret_owner)
    : buffer_(buffer),
      caret_offset_(text::Offset::Invalid()),
      layout_block_flow_(new LayoutBlockFlow(buffer)),
      layout_view_builder_(new LayoutViewBuilder(buffer, caret_owner)) {}

TextView::~TextView() {}

void TextView::DidChangeStyle(text::Offset offset, text::OffsetDelta length) {
  ASSERT_DOM_LOCKED();
  layout_block_flow_->DidChangeStyle(offset, length);
}

void TextView::DidDeleteAt(text::Offset offset, text::OffsetDelta length) {
  ASSERT_DOM_LOCKED();
  layout_block_flow_->DidDeleteAt(offset, length);
}

void TextView::DidHide() {
  view_paint_cache_.reset();
}

void TextView::DidInsertBefore(text::Offset offset, text::OffsetDelta length) {
  ASSERT_DOM_LOCKED();
  layout_block_flow_->DidInsertBefore(offset, length);
}

text::Offset TextView::EndOfLine(text::Offset text_offset) const {
  return layout_block_flow_->EndOfLine(text_offset);
}

text::Offset TextView::GetEnd() {
  return layout_block_flow_->GetEnd();
}

text::Offset TextView::GetStart() {
  return layout_block_flow_->GetStart();
}

text::Offset TextView::GetVisibleEnd() {
  return layout_block_flow_->GetVisibleEnd();
}

void TextView::Format(text::Offset text_offset) {
  layout_block_flow_->Format(text_offset);
}

bool TextView::FormatIfNeeded() {
  return layout_block_flow_->FormatIfNeeded();
}

// Maps specified buffer position to window point and returns true. If
// specified buffer point isn't in window, this function returns false.
//
// A TextView object must be formatted with the latest buffer.
//
gfx::RectF TextView::HitTestTextPosition(text::Offset text_offset) const {
  return layout_block_flow_->HitTestTextPosition(text_offset);
}

bool TextView::IsPositionFullyVisible(text::Offset offset) const {
  return layout_block_flow_->IsPositionFullyVisible(offset);
}

void TextView::MakeSelectionVisible() {
  // |UpdateAndPaint()| will format text view to place caret at selection
  // focus offset.
  caret_offset_ = text::Offset::Invalid();
}

text::Offset TextView::MapPointToPosition(gfx::PointF point) {
  return layout_block_flow_->MapPointToPosition(point);
}

text::Offset TextView::MapPointXToOffset(text::Offset text_offset,
                                         float point_x) const {
  return layout_block_flow_->MapPointXToOffset(text_offset, point_x);
}

void TextView::Paint(gfx::Canvas* canvas) {
  DCHECK(layout_view_);
  TRACE_EVENT0("view", "TextView::Paint");
  view_paint_cache_ = paint::ViewPainter(*layout_view_)
                          .Paint(canvas, std::move(view_paint_cache_));
}

bool TextView::ScrollDown() {
  return layout_block_flow_->ScrollDown();
}

void TextView::ScrollToPosition(text::Offset offset) {
  layout_block_flow_->ScrollToPosition(offset);
}

bool TextView::ScrollUp() {
  return layout_block_flow_->ScrollUp();
}

void TextView::SetBounds(const gfx::RectF& new_bounds) {
  DCHECK(!new_bounds.empty());
  if (bounds_.size() == new_bounds.size())
    return;
  bounds_ = new_bounds;
  view_paint_cache_.reset();
  layout_block_flow_->SetBounds(bounds_);
  layout_view_builder_->SetBounds(bounds_);
}

void TextView::SetZoom(float new_zoom) {
  DCHECK_GT(new_zoom, 0.0f);
  layout_block_flow_->SetZoom(new_zoom);
  layout_view_builder_->SetZoom(new_zoom);
}

text::Offset TextView::StartOfLine(text::Offset text_offset) const {
  return layout_block_flow_->StartOfLine(text_offset);
}

void TextView::Update(const TextSelectionModel& selection_model,
                      base::Time now) {
  UI_ASSERT_DOM_LOCKED();
  TRACE_EVENT0("view", "TextView::Update");
  auto const new_caret_offset =
      GetCaretOffset(buffer_, selection_model, caret_offset_);
  DCHECK(new_caret_offset.IsValid());

  if (FormatIfNeeded()) {
    if (caret_offset_ != new_caret_offset) {
      ScrollToPosition(new_caret_offset);
      caret_offset_ = new_caret_offset;
    }
  } else if (caret_offset_ != new_caret_offset) {
    caret_offset_ = new_caret_offset;
    if (!IsPositionFullyVisible(new_caret_offset))
      ScrollToPosition(new_caret_offset);
  }

  layout_view_ =
      layout_view_builder_->Build(*layout_block_flow_, selection_model, now);
}

// gfx::CanvasObserver
void TextView::DidRecreateCanvas() {
  view_paint_cache_.reset();
}

}  // namespace layout
