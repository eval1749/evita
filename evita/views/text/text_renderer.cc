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
namespace {

float AlignHeightToPixel(const gfx::Canvas& gfx, float height) {
  return gfx.AlignToPixel(gfx::SizeF(0.0f, height)).height;
}

float AlignWidthToPixel(const gfx::Canvas&, float width) {
  return width;
}

inline void drawLine(const gfx::Canvas& gfx, const gfx::Brush& brush,
                     float sx, float sy, float ex, float ey) {
  gfx.DrawLine(brush, sx, sy, ex, ey);
}

inline void drawVLine(const gfx::Canvas& gfx, const gfx::Brush& brush,
                      float x, float sy, float ey) {
  drawLine(gfx, brush, x, sy, x, ey);
}

} // namespace

using namespace rendering;

//////////////////////////////////////////////////////////////////////
//
// TextRenderer
//
TextRenderer::TextRenderer(text::Buffer* buffer)
    : canvas_(nullptr), m_pBuffer(buffer),
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
  TextFormatter oFormatter(*canvas_, text_block_.get(), lStart, zoom_);
  oFormatter.Format();
  should_format_ = false;
  should_render_ = true;
}

TextLine* TextRenderer::FormatLine(Posn lStart) {
  DCHECK(canvas_);
  TextFormatter oFormatter(*canvas_, text_block_.get(), lStart, zoom_);
  return oFormatter.FormatLine();
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

    auto lPosn = line->GetEnd() - 1;
    for (const auto cell : line->cells()) {
      auto x = pt.x - xCell;
      xCell += cell->width();
      auto lMap = cell->MapXToPosn(*canvas_, x);
      if (lMap >= 0)
        lPosn = lMap;
      if (x >= 0 && x < cell->width())
        break;
    }
    return lPosn;
  }
  return GetEnd() - 1;
}

// Returns number of lines to be displayed in this page when using
// buffer's default style.
int TextRenderer::pageLines() const {
  DCHECK(canvas_);
  DCHECK(!ShouldFormat());
  auto const pFont = FontSet::Get(*canvas_, m_pBuffer->GetDefaultStyle())->
        FindFont(*canvas_, 'x');
  auto const height = AlignHeightToPixel(*canvas_, pFont->height());
  return static_cast<int>(text_block_->height() / height);
}

void TextRenderer::Render(const TextSelectionModel& selection_model) {
  DCHECK(canvas_);
  DCHECK(!ShouldFormat());
  DCHECK(!text_block_->bounds().empty());
  text_block_->EnsureLinePoints();
  const auto selection = TextFormatter::FormatSelection(
      text_block_->text_buffer(), selection_model);
  screen_text_block_->Render(text_block_.get(), selection);

  // FIXME 2007-08-05 We should expose show/hide
  // ruler settings to both script and UI.

  // Ruler
  auto const pFont = FontSet::Get(*canvas_, m_pBuffer->GetDefaultStyle())->
    FindFont(*canvas_, 'x');

  // FIXME 2007-08-05 We should expose rule position to
  // user.
  auto const num_columns = 81;
  auto const width_of_M = AlignWidthToPixel(*canvas_, pFont->GetCharWidth('M'));
  drawVLine(*canvas_, gfx::Brush(*canvas_, gfx::ColorF::LightGray),
            text_block_->left() + width_of_M * num_columns,
            text_block_->top(), text_block_->bottom());
  should_render_ = false;
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
  auto const lStart = m_pBuffer->ComputeStartOfLine(lGoal);
  TextFormatter formatter(*canvas_, text_block_.get(), lStart, zoom_);
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

  should_render_ = true;
  return true;
}

bool TextRenderer::ScrollToPosition(Posn lPosn) {
  DCHECK(canvas_);
  DCHECK(!ShouldFormat());
  if (IsPositionFullyVisible(lPosn))
    return false;

  auto const cLines = pageLines();
  auto const cLines2 = std::max(cLines / 2, 1);

  if (lPosn > GetStart()) {
    for (auto k = 0; k < cLines2; k++) {
        if (!ScrollUp())
          return k;
        if (IsPositionFullyVisible(lPosn))
          return true;
    }
  } else {
    for (int k = 0; k < cLines2; k++) {
      if (!ScrollDown())
        return k;
      if (IsPositionFullyVisible(lPosn))
        return true;
    }
  }

  auto lStart = lPosn;
  for (int k = 0; k < cLines2; k++) {
    if (!lStart)
      break;
    lStart = m_pBuffer->ComputeStartOfLine(lStart - 1);
  }

  #if DEBUG_FORMAT
    DEBUG_PRINTF("%p\n", this);
  #endif // DEBUG_FORMAT

  Format(lStart);
  for (;;) {
    if (IsPositionFullyVisible(lPosn))
      break;
    if (!ScrollUp())
      break;
  }

  // If this page shows end of buffer, we shows lines as much as
  // possible to fit in page.
  if (GetEnd() >= m_pBuffer->GetEnd()) {
    while (IsPositionFullyVisible(lPosn)) {
      if (!ScrollDown())
        return true;
    }
    ScrollUp();
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

  TextFormatter oFormatter(*canvas_, text_block_.get(),
                           text_block_->GetLast()->GetEnd(), zoom_);

  auto const line = oFormatter.FormatLine();
  text_block_->Append(line);
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
