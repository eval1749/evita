// Copyright (c) 1996-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>
#include <memory>
#include <utility>
#include <vector>

#include "evita/views/text/text_view.h"

#include "base/trace_event/trace_event.h"
#include "evita/dom/lock.h"
#include "evita/editor/dom_lock.h"
#include "evita/text/buffer.h"
#include "evita/views/text/layout_view.h"
#include "evita/views/text/layout_block_flow.h"
#include "evita/views/text/layout_view_builder.h"
#include "evita/views/text/screen_text_block.h"
#include "evita/views/text/text_view_caret.h"

namespace views {
using namespace rendering;  // NOLINT

namespace {
text::Posn GetCaretOffset(const text::Buffer* buffer,
                          const TextSelectionModel& selection,
                          text::Posn caret_offset) {
  if (!selection.disabled())
    return selection.focus_offset();
  auto const max_offset = buffer->GetEnd();
  if (selection.start() == max_offset && selection.end() == max_offset)
    return max_offset;
  return caret_offset == -1 ? selection.focus_offset() : caret_offset;
}
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// TextView
//
TextView::TextView(text::Buffer* buffer, ui::CaretOwner* caret_owner)
    : buffer_(buffer),
      caret_(new TextViewCaret(caret_owner)),
      caret_offset_(-1),
      layout_block_flow_(new LayoutBlockFlow(buffer)),
      layout_view_builder_(new LayoutViewBuilder(buffer)),
      screen_text_block_(new ScreenTextBlock(caret_.get())) {}

TextView::~TextView() {}

void TextView::DidChangeStyle(text::Posn offset, size_t length) {
  ASSERT_DOM_LOCKED();
  layout_block_flow_->DidChangeStyle(offset, length);
}

void TextView::DidDeleteAt(text::Posn offset, size_t length) {
  ASSERT_DOM_LOCKED();
  layout_block_flow_->DidDeleteAt(offset, length);
}

void TextView::DidHide() {
  last_layout_view_ = nullptr;
}

void TextView::DidInsertAt(text::Posn offset, size_t length) {
  ASSERT_DOM_LOCKED();
  layout_block_flow_->DidInsertAt(offset, length);
}

void TextView::DidRecreateCanvas() {
  last_layout_view_ = nullptr;
}

text::Posn TextView::EndOfLine(text::Posn text_offset) const {
  return layout_block_flow_->EndOfLine(text_offset);
}

text::Posn TextView::GetEnd() {
  return layout_block_flow_->GetEnd();
}

text::Posn TextView::GetStart() {
  return layout_block_flow_->GetStart();
}

text::Posn TextView::GetVisibleEnd() {
  return layout_block_flow_->GetVisibleEnd();
}

void TextView::Format(text::Posn text_offset) {
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
gfx::RectF TextView::HitTestTextPosition(text::Posn text_offset) const {
  return layout_block_flow_->HitTestTextPosition(text_offset);
}

bool TextView::IsPositionFullyVisible(text::Posn offset) const {
  return layout_block_flow_->IsPositionFullyVisible(offset);
}

void TextView::MakeSelectionVisible() {
  // |UpdateAndPaint()| will format text view to place caret at selection
  // focus offset.
  caret_offset_ = -1;
}

text::Posn TextView::MapPointToPosition(gfx::PointF point) {
  return layout_block_flow_->MapPointToPosition(point);
}

text::Posn TextView::MapPointXToOffset(text::Posn text_offset,
                                       float point_x) const {
  return layout_block_flow_->MapPointXToOffset(text_offset, point_x);
}

void TextView::Paint(gfx::Canvas* canvas, base::Time now) {
  DCHECK(layout_view_);
  TRACE_EVENT0("view", "TextView::Paint");
  screen_text_block_->Paint(canvas, now, last_layout_view_.get(),
                            *layout_view_);
  if (canvas->screen_bitmap()) {
    last_layout_view_ = layout_view_;
    return;
  }
  last_layout_view_ = nullptr;
}

bool TextView::ScrollDown() {
  return layout_block_flow_->ScrollDown();
}

void TextView::ScrollToPosition(text::Posn offset) {
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
  caret_->Reset();
  last_layout_view_ = nullptr;
  layout_block_flow_->SetBounds(bounds_);
  layout_view_builder_->SetBounds(bounds_);
}

void TextView::SetZoom(float new_zoom) {
  DCHECK_GT(new_zoom, 0.0f);
  layout_block_flow_->SetZoom(new_zoom);
  layout_view_builder_->SetZoom(new_zoom);
}

text::Posn TextView::StartOfLine(text::Posn text_offset) const {
  return layout_block_flow_->StartOfLine(text_offset);
}

void TextView::Update(const TextSelectionModel& selection_model) {
  UI_ASSERT_DOM_LOCKED();
  TRACE_EVENT0("view", "TextView::Update");
  auto const new_caret_offset =
      GetCaretOffset(buffer_, selection_model, caret_offset_);
  DCHECK_GE(new_caret_offset, 0);

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
      layout_view_builder_->Build(*layout_block_flow_, selection_model);
}

}  // namespace views
