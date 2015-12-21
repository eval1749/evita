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
#include "evita/layout/block_flow.h"
#include "evita/layout/paint_view_builder.h"
#include "evita/paint/public/selection.h"
#include "evita/paint/public/view.h"
#include "evita/paint/view_painter.h"
#include "evita/paint/view_paint_cache.h"
#include "evita/text/buffer.h"
#include "evita/text/static_range.h"

namespace layout {

namespace {
text::Offset GetCaretOffset(const text::Buffer& buffer,
                            const TextSelectionModel& selection,
                            text::Offset caret_offset) {
  if (!selection.disabled())
    return selection.focus_offset();
  auto const max_offset = buffer.GetEnd();
  if (selection.start() == max_offset && selection.end() == max_offset)
    return max_offset;
  return caret_offset.IsValid() ? caret_offset : selection.focus_offset();
}
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// TextView
//
TextView::TextView(const text::Buffer& buffer,
                   ui::AnimatableWindow* caret_owner)
    : block_(new BlockFlow(buffer)),
      buffer_(buffer),
      caret_offset_(text::Offset::Invalid()),
      paint_view_builder_(new PaintViewBuilder(*block_, caret_owner)) {}

TextView::~TextView() {}

text::Offset TextView::text_end() const {
  DCHECK(!block_->NeedsFormat());
  return block_->text_end();
}

text::Offset TextView::text_start() const {
  DCHECK(!block_->NeedsFormat());
  return block_->text_start();
}

text::Offset TextView::ComputeEndOfLine(text::Offset text_offset) const {
  return block_->ComputeEndOfLine(text_offset);
}

text::Offset TextView::ComputeStartOfLine(text::Offset text_offset) const {
  return block_->ComputeStartOfLine(text_offset);
}

text::Offset TextView::ComputeVisibleEnd() const {
  return block_->ComputeVisibleEnd();
}

void TextView::DidChangeStyle(const text::StaticRange& range) {
  ASSERT_DOM_LOCKED();
  block_->DidChangeStyle(range);
}

void TextView::DidDeleteAt(const text::StaticRange& range) {
  ASSERT_DOM_LOCKED();
  block_->DidDeleteAt(range);
}

void TextView::DidHide() {
  view_paint_cache_.reset();
}

void TextView::DidInsertBefore(const text::StaticRange& range) {
  ASSERT_DOM_LOCKED();
  block_->DidInsertBefore(range);
}

void TextView::Format(text::Offset text_offset) {
  block_->Format(text_offset);
}

bool TextView::FormatIfNeeded() {
  return block_->FormatIfNeeded();
}

// Maps specified buffer position to window point and returns true. If
// specified buffer point isn't in window, this function returns false.
//
// A TextView object must be formatted with the latest buffer.
//
gfx::RectF TextView::HitTestTextPosition(text::Offset text_offset) const {
  return block_->HitTestTextPosition(text_offset);
}

void TextView::MakeSelectionVisible() {
  // |UpdateAndPaint()| will format text view to place caret at selection
  // focus offset.
  caret_offset_ = text::Offset::Invalid();
}

text::Offset TextView::HitTestPoint(gfx::PointF point) {
  return block_->HitTestPoint(point);
}

text::Offset TextView::MapPointXToOffset(text::Offset text_offset,
                                         float point_x) const {
  return block_->MapPointXToOffset(text_offset, point_x);
}

void TextView::Paint(gfx::Canvas* canvas) {
  DCHECK(paint_view_);
  TRACE_EVENT0("view", "TextView::Paint");
  view_paint_cache_ = paint::ViewPainter(*paint_view_)
                          .Paint(canvas, std::move(view_paint_cache_));
}

bool TextView::ScrollDown() {
  return block_->ScrollDown();
}

void TextView::ScrollToPosition(text::Offset offset) {
  block_->ScrollToPosition(offset);
}

bool TextView::ScrollUp() {
  return block_->ScrollUp();
}

void TextView::SetBounds(const gfx::RectF& new_bounds) {
  DCHECK(!new_bounds.empty());
  if (bounds_.size() == new_bounds.size())
    return;
  bounds_ = new_bounds;
  view_paint_cache_.reset();
  block_->SetBounds(bounds_);
  paint_view_builder_->SetBounds(bounds_);
}

void TextView::SetZoom(float new_zoom) {
  DCHECK_GT(new_zoom, 0.0f);
  block_->SetZoom(new_zoom);
  paint_view_builder_->SetZoom(new_zoom);
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
    if (!block_->IsFullyVisibleTextPosition(new_caret_offset))
      ScrollToPosition(new_caret_offset);
  }

  paint_view_ = paint_view_builder_->Build(selection_model, now);
}

// gfx::CanvasObserver
void TextView::DidRecreateCanvas() {
  view_paint_cache_.reset();
}

}  // namespace layout
