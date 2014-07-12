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

#include "evita/gfx_base.h"
#include "evita/text/buffer.h"
#include "evita/views/text/render_cell.h"
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
    : buffer_(buffer), canvas_(nullptr),
      screen_text_block_(new ScreenTextBlock()),
      should_format_(true), should_render_(true),
      text_block_(new TextBlock(buffer)), zoom_(1.0f) {
}

TextRenderer::~TextRenderer() {
}

void TextRenderer::set_zoom(float new_zoom) {
  DCHECK_GT(new_zoom, 0.0f);
  if (zoom_ == new_zoom)
    return;
  zoom_ = new_zoom;
  should_format_ = true;
}

void TextRenderer::DidKillFocus() {
  screen_text_block_->DidKillFocus();
}

void TextRenderer::DidSetFocus() {
  screen_text_block_->DidSetFocus();
}

TextLine* TextRenderer::FindLine(Posn lPosn) const {
  DCHECK(!ShouldFormat());
  if (lPosn < GetStart() || lPosn > GetEnd())
    return nullptr;

  for (auto const line : text_block_->lines()) {
    if (lPosn < line->text_end())
      return line;
  }

  // We must not be here.
  return nullptr;
}

text::Posn TextRenderer::GetEnd() const {
  DCHECK(!ShouldFormat());
  return text_block_->GetLast()->GetEnd();
}

text::Posn TextRenderer::GetStart() const {
  DCHECK(!ShouldFormat());
  return text_block_->GetFirst()->GetStart();
}

text::Posn TextRenderer::GetVisibleEnd() const {
  DCHECK(!ShouldFormat());
  return text_block_->GetVisibleEnd();
}

void TextRenderer::Format(Posn lStart) {
  DCHECK(canvas_);
  text_block_->Reset();
  TextFormatter formatter(canvas_, text_block_.get(), lStart, zoom_);
  formatter.Format();
  should_format_ = false;
  should_render_ = true;
}

TextLine* TextRenderer::FormatLine(Posn lStart) {
  DCHECK(canvas_);
  TextFormatter formatter(canvas_, text_block_.get(), lStart, zoom_);
  return formatter.FormatLine();
}

// Maps specified buffer position to window point and returns true. If
// specified buffer point isn't in window, this function returns false.
//
// A TextRenderer object must be formatted with the latest buffer.
//
gfx::RectF TextRenderer::HitTestTextPosition(Posn lPosn) const {
  DCHECK(!ShouldFormat());
  if (lPosn < GetStart() || lPosn > GetEnd())
    return gfx::RectF();
  for (auto const line : text_block_->lines()) {
    auto const rect = line->HitTestTextPosition(lPosn);
    if (!rect.empty())
      return rect;
  }
  return gfx::RectF();
}

bool TextRenderer::IsPositionFullyVisible(text::Posn offset) const {
  DCHECK(!ShouldFormat());
  return offset >= GetStart() && offset < GetVisibleEnd();
}

Posn TextRenderer::MapPointToPosition(gfx::PointF pt) const {
  DCHECK(canvas_);
  DCHECK(!ShouldFormat());
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
      const auto offset = cell->MapXToPosn(canvas_, x);
      if (offset >= 0)
        result_offset = offset;
      if (x >= 0 && x < cell->width())
        break;
    }
    return result_offset;
  }
  return GetEnd() - 1;
}

void TextRenderer::Render(const TextSelectionModel& selection_model) {
  DCHECK(canvas_);
  DCHECK(!ShouldFormat());
  DCHECK(!text_block_->bounds().empty());
  text_block_->EnsureLinePoints();
  const auto selection = TextFormatter::FormatSelection(
      text_block_->text_buffer(), selection_model);
  screen_text_block_->Render(text_block_.get(), selection);
  RenderRuler();
  should_render_ = false;
}

void TextRenderer::RenderRuler() {
  // FIXME 2007-08-05 We should expose show/hide and ruler settings to both
  // script and UI.
  auto style = buffer_->GetDefaultStyle();
  style.set_font_size(style.font_size() * zoom_);
  auto const font = FontSet::Get(style)->FindFont('x');

  auto const num_columns = 81;
  auto const width_of_M = font->GetCharWidth('M');
  auto const ruler_x = ::floor(text_block_->left() + width_of_M * num_columns);
  gfx::RectF ruler_bounds(gfx::PointF(ruler_x, text_block_->top()),
                          gfx::SizeF(1.0f, text_block_->height()));

  gfx::Canvas::AxisAlignedClipScope clip_scope(*canvas_, ruler_bounds);
  gfx::Brush brush(canvas_, gfx::ColorF(0, 0, 0, 0.3f));
  canvas_->DrawRectangle(brush, ruler_bounds);
}

void TextRenderer::RenderSelectionIfNeeded(
    const TextSelectionModel& new_selection_model) {
  DCHECK(!ShouldFormat());
  DCHECK(!should_render_);
  DCHECK(!text_block_->bounds().empty());
  screen_text_block_->RenderSelectionIfNeeded(
      TextFormatter::FormatSelection(text_block_->text_buffer(),
                                     new_selection_model));
}

void TextRenderer::Reset() {
  screen_text_block_->Reset();
}

bool TextRenderer::ScrollDown() {
  DCHECK(canvas_);
  DCHECK(!ShouldFormat());
  if (!GetStart())
    return false;
  auto const lGoal = GetStart() - 1;
  auto const lStart = buffer_->ComputeStartOfLine(lGoal);
  TextFormatter formatter(canvas_, text_block_.get(), lStart, zoom_);
  for (;;) {
    auto const line = formatter.FormatLine();
    if (lGoal < line->GetEnd()) {
      text_block_->Prepend(line);
      break;
    }
  }

  // Discard lines outside of screen.
  text_block_->EnsureLinePoints();
  while (text_block_->GetLast()->top() >= text_block_->bottom()) {
    text_block_->DiscardLastLine();
  }

  should_format_ = false;
  should_render_ = true;
  return true;
}

bool TextRenderer::ScrollToPosition(Posn offset) {
  DCHECK(canvas_);
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

  Format(buffer_->ComputeStartOfLine(offset));
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
  DCHECK(canvas_);
  DCHECK(!ShouldFormat());
  text_block_->EnsureLinePoints();
  if (text_block_->IsShowEndOfDocument())
    return false;

  if (!text_block_->DiscardFirstLine())
    return false;

  text_block_->EnsureLinePoints();
  if (text_block_->IsShowEndOfDocument())
    return false;

  TextFormatter formatter(canvas_, text_block_.get(),
                          text_block_->GetLast()->GetEnd(), zoom_);

  auto const line = formatter.FormatLine();
  text_block_->Append(line);
  should_format_ = false;
  should_render_ = true;
  return true;
}

void TextRenderer::SetBounds(const Rect& rect) {
  gfx::RectF bounds(rect);
  text_block_->SetBounds(bounds);
  screen_text_block_->SetBounds(bounds);
  should_format_ = true;
  should_render_ = true;
}

void TextRenderer::SetCanvas(gfx::Canvas* canvas) {
  canvas_ = canvas;
  screen_text_block_->SetCanvas(canvas);
  should_format_ = true;
  should_render_ = true;
}

bool TextRenderer::ShouldFormat() const {
  return should_format_ || text_block_->dirty();
}

bool TextRenderer::ShouldRender() const {
  return should_render_ || screen_text_block_->dirty();
}

}  // namespace views
