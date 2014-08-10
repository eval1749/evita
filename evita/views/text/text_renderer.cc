// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#define DEBUG_DIRTY  0
#define DEBUG_DISPBUF 0
#define DEBUG_FORMAT 0
#define DEBUG_RENDER 0
#include "evita/views/text/text_renderer.h"

#include <algorithm>
#include <memory>
#include <utility>

#include "evita/dom/lock.h"
#include "evita/editor/dom_lock.h"
#include "evita/gfx_base.h"
#include "evita/text/buffer.h"
#include "evita/views/text/render_cell.h"
#include "evita/views/text/render_font.h"
#include "evita/views/text/render_font_set.h"
#include "evita/views/text/render_text_block.h"
#include "evita/views/text/render_text_line.h"
#include "evita/views/text/screen_text_block.h"
#include "evita/views/text/text_formatter.h"

namespace views {
using namespace rendering;

//////////////////////////////////////////////////////////////////////
//
// TextRenderer
//
TextRenderer::TextRenderer(text::Buffer* buffer)
    : buffer_(buffer), screen_text_block_(new ScreenTextBlock()),
      should_render_(true), text_block_(new TextBlock(buffer)),
      view_start_(0), zoom_(1.0f) {
  buffer_->AddObserver(this);
}

TextRenderer::~TextRenderer() {
  buffer()->RemoveObserver(this);
}

void TextRenderer::set_zoom(float new_zoom) {
  DCHECK_GT(new_zoom, 0.0f);
  if (zoom_ == new_zoom)
    return;
  zoom_ = new_zoom;
}

void TextRenderer::DidHide() {
  screen_text_block_->Reset();
}

void TextRenderer::DidKillFocus(gfx::Canvas* canvas) {
  screen_text_block_->DidKillFocus(canvas);
}

void TextRenderer::DidLostCanvas() {
  screen_text_block_->Reset();
}

void TextRenderer::DidSetFocus() {
  screen_text_block_->DidSetFocus();
}

// Returns end of line offset containing |text_offset|.
text::Posn TextRenderer::EndOfLine(text::Posn text_offset) const {
  UI_ASSERT_DOM_LOCKED();

  if (text_offset >= buffer()->GetEnd())
    return buffer()->GetEnd();

  if (!ShouldFormat()) {
    if (auto const line = FindLine(text_offset))
      return line->GetEnd() - 1;
  }

  auto start_offset = buffer()->ComputeStartOfLine(text_offset);
  TextFormatter formatter(buffer_, start_offset, bounds_, zoom_);
  for (;;) {
    std::unique_ptr<TextLine> line(formatter.FormatLine());
    if (text_offset < line->GetEnd())
      return line->GetEnd() - 1;
  }
}

TextLine* TextRenderer::FindLine(Posn lPosn) const {
  DCHECK(!ShouldFormat());
  if (lPosn < text_block_->GetFirst()->GetStart() ||
      lPosn > text_block_->GetFirst()->GetEnd()) {
    return nullptr;
  }
  for (auto const line : text_block_->lines()) {
    if (lPosn < line->text_end())
      return line;
  }
  return nullptr;
}

text::Posn TextRenderer::GetEnd() {
  FormatIfNeeded();
  return text_block_->GetLast()->GetEnd();
}

text::Posn TextRenderer::GetStart() {
  FormatIfNeeded();
  return text_block_->GetFirst()->GetStart();
}

text::Posn TextRenderer::GetVisibleEnd() {
  FormatIfNeeded();
  return text_block_->GetVisibleEnd();
}

void TextRenderer::Format(text::Posn text_offset) {
  text_block_->Format(text_offset, bounds_, zoom_);
  view_start_ = text_block_->GetFirst()->GetStart();
  should_render_ = true;
}

bool TextRenderer::FormatIfNeeded() {
  if (!ShouldFormat())
    return false;
  Format(view_start_);
  return true;
}

// Maps specified buffer position to window point and returns true. If
// specified buffer point isn't in window, this function returns false.
//
// A TextRenderer object must be formatted with the latest buffer.
//
gfx::RectF TextRenderer::HitTestTextPosition(text::Posn text_offset) const {
  if (!ShouldFormat()) {
    if (auto const line = FindLine(text_offset))
      return line->HitTestTextPosition(text_offset);
  }

  auto start_offset = buffer()->ComputeStartOfLine(text_offset);
  TextFormatter formatter(buffer_, start_offset, bounds_, zoom_);
  for (;;) {
    std::unique_ptr<TextLine> line(formatter.FormatLine());
    if (text_offset < line->GetEnd())
      return line->HitTestTextPosition(text_offset);
  }
}

bool TextRenderer::IsPositionFullyVisible(text::Posn offset) const {
  DCHECK(!ShouldFormat());
  return offset >= text_block_->GetFirst()->GetStart() &&
         offset < text_block_->GetVisibleEnd();
}

text::Posn TextRenderer::MapPointToPosition(gfx::PointF pt) {
  FormatIfNeeded();
  if (pt.y < text_block_->top())
    return GetStart();
  if (pt.y >= text_block_->bottom())
    return GetEnd();

  auto yLine = text_block_->top();
  for (const auto line : text_block_->lines()) {
    auto const y = pt.y - yLine;
    yLine += line->GetHeight();

    if (y >= line->GetHeight())
      continue;

    auto xCell = text_block_->left();
    if (pt.x < xCell)
      return line->GetStart();

    auto result_offset = line->GetEnd() - 1;
    for (const auto cell : line->cells()) {
      auto x = pt.x - xCell;
      xCell += cell->width();
      const auto offset = cell->MapXToPosn(x);
      if (offset >= 0)
        result_offset = offset;
      if (x >= 0 && x < cell->width())
        break;
    }
    return result_offset;
  }
  return GetEnd() - 1;
}

text::Posn TextRenderer::MapPointXToOffset(text::Posn text_offset,
                                           float point_x) const {
  if (!ShouldFormat()) {
    if (auto const line = FindLine(text_offset))
      return line->MapXToPosn(point_x);
  }

  auto start_offset = buffer()->ComputeStartOfLine(text_offset);
  TextFormatter formatter(buffer_, start_offset, bounds_, zoom_);
  for (;;) {
    std::unique_ptr<TextLine> line(formatter.FormatLine());
    if (text_offset < line->GetEnd())
      return line->MapXToPosn(point_x);
  }
}

void TextRenderer::Render(gfx::Canvas* canvas,
                          const TextSelectionModel& selection_model) {
  DCHECK(!ShouldFormat());
  DCHECK(!text_block_->bounds().empty());
  text_block_->EnsureLinePoints();
  const auto selection = TextFormatter::FormatSelection(buffer_,
                                                        selection_model);
  screen_text_block_->Render(canvas, text_block_.get(), selection);
  RenderRuler(canvas);
  should_render_ = false;
}

void TextRenderer::RenderRuler(gfx::Canvas* canvas) {
  // FIXME 2007-08-05 We should expose show/hide and ruler settings to both
  // script and UI.
  auto style = buffer_->GetDefaultStyle();
  style.set_font_size(style.font_size() * zoom_);
  auto const font = FontSet::GetFont(style, 'x');

  auto const num_columns = 81;
  auto const width_of_M = font->GetCharWidth('M');
  auto const ruler_x = ::floor(text_block_->left() + width_of_M * num_columns);
  gfx::RectF ruler_bounds(gfx::PointF(ruler_x, text_block_->top()),
                          gfx::SizeF(1.0f, text_block_->height()));

  gfx::Canvas::AxisAlignedClipScope clip_scope(canvas, ruler_bounds);
  gfx::Brush brush(canvas, gfx::ColorF(0, 0, 0, 0.3f));
  canvas->DrawRectangle(brush, ruler_bounds);
}

void TextRenderer::RenderSelectionIfNeeded(
    gfx::Canvas* canvas,
    const TextSelectionModel& new_selection_model) {
  DCHECK(!ShouldFormat());
  DCHECK(!should_render_);
  DCHECK(!text_block_->bounds().empty());
  screen_text_block_->RenderSelectionIfNeeded(canvas,
      TextFormatter::FormatSelection(buffer_, new_selection_model));
}

bool TextRenderer::ScrollDown() {
  DCHECK(!ShouldFormat());
  if (!GetStart())
    return false;
  auto const goal_offset = GetStart() - 1;
  auto const start_offset = buffer_->ComputeStartOfLine(goal_offset);
  TextFormatter formatter(buffer_, start_offset, bounds_, zoom_);
  for (;;) {
    auto const line = formatter.FormatLine();
    if (goal_offset < line->GetEnd()) {
      text_block_->Prepend(line);
      break;
    }
  }

  // Discard lines outside of screen.
  text_block_->EnsureLinePoints();
  view_start_ = text_block_->GetFirst()->GetStart();
  while (text_block_->GetLast()->top() >= text_block_->bottom()) {
    text_block_->DiscardLastLine();
  }

  should_render_ = true;
  return true;
}

bool TextRenderer::ScrollToPosition(Posn offset) {
  DCHECK(!ShouldFormat());
  if (IsPositionFullyVisible(offset))
    return false;

  const auto scrollable = text_block_->height() / 2;

  if (offset > GetStart()) {
    auto scrolled = 0.0f;
    while (scrolled < scrollable) {
      const auto scroll_height = text_block_->GetFirst()->height();
      if (!ScrollUp())
        return scrolled > 0.0f;
      if (IsPositionFullyVisible(offset))
        return true;
      scrolled += scroll_height;
    }
  } else {
    auto scrolled = 0.0f;
    while (scrolled < scrollable) {
      auto const scroll_height = text_block_->GetLast()->height();
      if (!ScrollDown())
        return scrolled > 0.0f;
      if (IsPositionFullyVisible(offset))
        return true;
      scrolled += scroll_height;
    }
  }

  Format(offset);
  while (!IsPositionFullyVisible(offset)) {
    if (!ScrollDown())
      return true;
  }

  // If this page shows end of buffer, we shows lines as much as
  // possible to fit in page.
  if (GetEnd() >= buffer_->GetEnd()) {
    while (IsPositionFullyVisible(offset)) {
      if (!ScrollDown())
        return true;
    }
    ScrollUp();
    return true;
  }

  // Move line containing |offset| to middle of screen.
  auto scrolled = HitTestTextPosition(offset).top - text_block_->top();
  if (scrolled < scrollable) {
    while (scrolled < scrollable) {
      auto const scroll_height = text_block_->GetFirst()->height();
      if (!ScrollDown())
        return true;
      scrolled += scroll_height;
    }
  } else {
    scrolled = scrollable + scrolled;
    while (scrolled < scrollable) {
      auto const scroll_height = text_block_->GetLast()->height();
      if (!ScrollUp())
        return true;
      scrolled += scroll_height;
    }
  }

  return true;
}

bool TextRenderer::ScrollUp() {
  DCHECK(!ShouldFormat());
  text_block_->EnsureLinePoints();
  if (text_block_->IsShowEndOfDocument())
    return false;

  if (!text_block_->DiscardFirstLine())
    return false;

  text_block_->EnsureLinePoints();
  view_start_ = text_block_->GetFirst()->GetStart();
  if (text_block_->IsShowEndOfDocument())
    return false;

  auto const start_offset = text_block_->GetLast()->GetEnd();
  TextFormatter formatter(buffer_, start_offset, bounds_, zoom_);
  auto const line = formatter.FormatLine();
  text_block_->Append(line);
  should_render_ = true;
  return true;
}

void TextRenderer::SetBounds(const gfx::RectF& new_bounds) {
  DCHECK(!new_bounds.empty());
  if (bounds_.size() == new_bounds.size())
    return;
  bounds_ = new_bounds;
  screen_text_block_->SetBounds(bounds_);
  should_render_ = true;
}

bool TextRenderer::ShouldFormat() const {
  UI_ASSERT_DOM_LOCKED();
  return text_block_->ShouldFormat(bounds_, zoom_);
}

bool TextRenderer::ShouldRender() const {
  return should_render_ || screen_text_block_->dirty();
}
// Returns end of line offset containing |text_offset|.
text::Posn TextRenderer::StartOfLine(text::Posn text_offset) const {
  UI_ASSERT_DOM_LOCKED();

  if (text_offset <= 0)
    return 0;

  if (!ShouldFormat()) {
    if (auto const line = FindLine(text_offset))
      return line->GetStart();
  }

  auto start_offset = buffer()->ComputeStartOfLine(text_offset);
  if (!start_offset)
    return 0;

  TextFormatter formatter(buffer_, start_offset, bounds_, zoom_);
  for (;;) {
    std::unique_ptr<TextLine> line(formatter.FormatLine());
    start_offset = line->GetEnd();
    if (text_offset < start_offset)
      return line->GetStart();
  }
}

// text::BufferMutationObserver
void TextRenderer::DidDeleteAt(text::Posn offset, size_t length) {
  ASSERT_DOM_LOCKED();
  if (view_start_ <= offset)
    return;
  view_start_ = std::max(static_cast<text::Posn>(view_start_ - length),
                         offset);
}

void TextRenderer::DidInsertAt(text::Posn offset, size_t length) {
  ASSERT_DOM_LOCKED();
  if (view_start_ <= offset)
    return;
  view_start_ += length;
}

}  // namespace views
