// Copyright (c) 1996-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>
#include <memory>
#include <utility>
#include <vector>

#include "evita/layout/text_view.h"

#include "base/trace_event/trace_event.h"
#include "evita/layout/block_flow.h"
#include "evita/paint/public/selection.h"
#include "evita/paint/public/view.h"
#include "evita/paint/view_painter.h"
#include "evita/text/buffer.h"
#include "evita/text/static_range.h"

namespace layout {

namespace {

#if 0
text::Offset GetCaretOffset(const text::Buffer& buffer,
                            const TextSelectionModel& selection,
                            text::Offset caret_offset) {
  if (!selection.disabled())
    return selection.focus_offset();
  auto const max_offset = buffer.GetEnd();
  if (selection.start() == max_offset && selection.end() == max_offset)
    return max_offset;
  return caret_offset.IsValid() ? caret_offset : selection.focus_offset();
  return selection.focus_offset();
}
#endif

gfx::RectF RoundBounds(const gfx::RectF& bounds) {
  return gfx::RectF(::floor(bounds.left), ::floor(bounds.top),
                    ::floor(bounds.right), ::floor(bounds.bottom));
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// TextView
//
TextView::TextView(const text::Buffer& buffer)
    : block_(new BlockFlow(buffer)),
      buffer_(buffer),
      caret_offset_(text::Offset::Invalid()) {}

TextView::~TextView() {}

text::Offset TextView::text_end() const {
  DCHECK(!block_->NeedsFormat());
  return block_->text_end();
}

text::Offset TextView::text_start() const {
  DCHECK(!block_->NeedsFormat());
  return block_->text_start();
}

float TextView::zoom() const {
  return block_->zoom();
}

text::Offset TextView::ComputeEndOfLine(text::Offset text_offset) const {
  return block_->ComputeEndOfLine(text_offset);
}

gfx::RectF TextView::ComputeCaretBounds(
    const TextSelectionModel& selection) const {
  if (!selection.has_focus())
    return gfx::RectF();
  const auto& char_rect =
      RoundBounds(block_->HitTestTextPosition(selection.focus_offset()));
  if (char_rect.empty())
    return gfx::RectF();
  // TODO(eval1749): Height of caret should be height of inserted character
  // instead of height of character after caret.
  const auto caret_width = 2;
  return gfx::RectF(char_rect.left, char_rect.top, char_rect.left + caret_width,
                    char_rect.bottom);
}

text::Offset TextView::ComputeStartOfLine(text::Offset text_offset) const {
  return block_->ComputeStartOfLine(text_offset);
}

text::Offset TextView::ComputeVisibleEnd() const {
  return block_->ComputeVisibleEnd();
}

void TextView::DidChangeStyle(const text::StaticRange& range) {
  block_->DidChangeStyle(range);
}

void TextView::DidDeleteAt(const text::StaticRange& range) {
  block_->DidDeleteAt(range);
}

void TextView::DidInsertBefore(const text::StaticRange& range) {
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
  // Next call of |Update()| will show caret in viewport.
  caret_offset_ = text::Offset::Invalid();
}

text::Offset TextView::HitTestPoint(gfx::PointF point) {
  return block_->HitTestPoint(point);
}

text::Offset TextView::MapPointXToOffset(text::Offset text_offset,
                                         float point_x) const {
  return block_->MapPointXToOffset(text_offset, point_x);
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
  block_->SetBounds(new_bounds);
}

void TextView::SetZoom(float new_zoom) {
  DCHECK_GT(new_zoom, 0.0f);
  block_->SetZoom(new_zoom);
}

void TextView::Update(const TextSelectionModel& selection_model) {
  TRACE_EVENT0("view", "TextView::Update");
  auto const new_caret_offset = selection_model.focus_offset();
  DCHECK(new_caret_offset.IsValid());
  FormatIfNeeded();
  if (caret_offset_ == new_caret_offset)
    return;
  caret_offset_ = new_caret_offset;
  if (block_->IsFullyVisibleTextPosition(new_caret_offset))
    return;
  ScrollToPosition(new_caret_offset);
}

}  // namespace layout
