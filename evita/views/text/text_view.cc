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
#include "evita/gfx_base.h"
#include "evita/text/buffer.h"
#include "evita/views/text/layout_view.h"
#include "evita/views/text/render_font.h"
#include "evita/views/text/render_font_set.h"
#include "evita/views/text/layout_block_flow.h"
#include "evita/views/text/root_inline_box.h"
#include "evita/views/text/screen_text_block.h"
#include "evita/views/text/text_formatter.h"

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
      caret_offset_(-1),
      format_counter_(0),
      layout_block_flow_(new LayoutBlockFlow(buffer)),
      screen_text_block_(new ScreenTextBlock(caret_owner)),
      should_paint_(true),
      zoom_(1.0f) {}

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
  screen_text_block_->Reset();
}

void TextView::DidInsertAt(text::Posn offset, size_t length) {
  ASSERT_DOM_LOCKED();
  layout_block_flow_->DidInsertAt(offset, length);
}

void TextView::DidRecreateCanvas() {
  screen_text_block_->Reset();
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
  should_paint_ = true;
}

bool TextView::FormatIfNeeded() {
  if (!layout_block_flow_->FormatIfNeeded() &&
      format_counter_ == layout_block_flow_->format_counter()) {
    return false;
  }
  should_paint_ = true;
  return true;
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
  if (!should_paint_ && canvas->screen_bitmap()) {
    screen_text_block_->PaintSelectionIfNeeded(canvas,
                                               layout_view_->selection(), now);
    return;
  }
  screen_text_block_->Paint(canvas, *layout_view_, now);
  PaintRuler(canvas);
  format_counter_ = layout_block_flow_->format_counter();
  should_paint_ = false;
}

void TextView::PaintRuler(gfx::Canvas* canvas) {
  // TODO(eval1749): We should expose show/hide and ruler settings to both
  // script and UI.
  auto style = buffer_->GetDefaultStyle();
  style.set_font_size(style.font_size() * zoom_);
  auto const font = FontSet::GetFont(style, 'x');

  auto const num_columns = 81;
  auto const width_of_M = font->GetCharWidth('M');
  auto const ruler_x = ::floor(bounds_.left + width_of_M * num_columns);
  auto const ruler_bounds = gfx::RectF(gfx::PointF(ruler_x, bounds_.top),
                                       gfx::SizeF(1.0f, bounds_.height()));

  gfx::Canvas::AxisAlignedClipScope clip_scope(canvas, ruler_bounds);
  gfx::Brush brush(canvas, gfx::ColorF(0, 0, 0, 0.3f));
  canvas->DrawRectangle(brush, ruler_bounds);
}

bool TextView::ScrollDown() {
  if (!layout_block_flow_->ScrollDown())
    return false;
  should_paint_ = true;
  return true;
}

void TextView::ScrollToPosition(text::Posn offset) {
  if (!layout_block_flow_->ScrollToPosition(offset))
    return;
  should_paint_ = true;
}

bool TextView::ScrollUp() {
  if (!layout_block_flow_->ScrollUp())
    return false;
  should_paint_ = true;
  return true;
}

void TextView::SetBounds(const gfx::RectF& new_bounds) {
  DCHECK(!new_bounds.empty());
  if (bounds_.size() == new_bounds.size())
    return;
  bounds_ = new_bounds;
  layout_block_flow_->SetBounds(bounds_);
  screen_text_block_->SetBounds(bounds_);
  should_paint_ = true;
}

void TextView::SetZoom(float new_zoom) {
  DCHECK_GT(new_zoom, 0.0f);
  if (zoom_ == new_zoom)
    return;
  zoom_ = new_zoom;
  layout_block_flow_->SetZoom(zoom_);
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

  const auto selection =
      TextFormatter::FormatSelection(buffer_, selection_model);
  // TODO(eval1749): We should recompute default style when style is changed,
  // rather than every |Format| call.
  const auto& style = buffer_->GetDefaultStyle();
  std::vector<RootInlineBox*> lines;
  for (const auto& line : layout_block_flow_->lines())
    lines.push_back(line->Copy());
  layout_view_.reset(
      new LayoutView(lines, selection, ColorToColorF(style.bgcolor())));
}

}  // namespace views
