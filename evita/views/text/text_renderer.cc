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

float AlignHeightToPixel(const gfx::Graphics& gfx, float height) {
  return gfx.AlignToPixel(gfx::SizeF(0.0f, height)).height;
}

float AlignWidthToPixel(const gfx::Graphics&, float width) {
  return width;
}

inline void drawLine(const gfx::Graphics& gfx, const gfx::Brush& brush,
                     float sx, float sy, float ex, float ey) {
  gfx.DrawLine(brush, sx, sy, ex, ey);
}

inline void drawVLine(const gfx::Graphics& gfx, const gfx::Brush& brush,
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
    : gfx_(nullptr),
      m_pBuffer(buffer),
      screen_text_block_(new ScreenTextBlock()),
      text_block_(new TextBlock(buffer)) {
}

TextRenderer::~TextRenderer() {
}

TextLine* TextRenderer::FindLine(Posn lPosn) const {
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
  DCHECK(!text_block_->dirty());
  return text_block_->GetLast()->GetEnd();
}

text::Posn TextRenderer::GetStart() const {
  DCHECK(!text_block_->dirty());
  return text_block_->GetFirst()->GetStart();
}

text::Posn TextRenderer::GetVisibleEnd() const {
  DCHECK(!text_block_->dirty());
  return text_block_->GetVisibleEnd();
}

void TextRenderer::Format(Posn lStart) {
  DCHECK(gfx_);
  text_block_->Reset();
  TextFormatter oFormatter(*gfx_, text_block_.get(), selection_, lStart);
  oFormatter.Format();
}

TextLine* TextRenderer::FormatLine(Posn lStart) {
  DCHECK(gfx_);
  TextFormatter oFormatter(*gfx_, text_block_.get(), selection_, lStart);
  return oFormatter.FormatLine();
}

// Maps specified buffer position to window point and returns true. If
// specified buffer point isn't in window, this function returns false.
//
// A TextRenderer object must be formatted with the latest buffer.
//
gfx::RectF TextRenderer::HitTestTextPosition(Posn lPosn) const {
  if (lPosn < GetStart() || lPosn > GetEnd())
    return gfx::RectF();
  for (auto const line : text_block_->lines()) {
    auto const rect = line->HitTestTextPosition(lPosn);
    if (!rect.empty())
      return rect;
  }
  return gfx::RectF();
}

bool TextRenderer::isPosnVisible(Posn lPosn) const {
  if (lPosn < GetStart())
    return false;
  if (lPosn >= GetEnd())
    return false;

  auto y = text_block_->top();
  for (const auto& line : text_block_->lines()) {
    if (lPosn >= line->GetStart() && lPosn < line->GetEnd())
      return y + line->GetHeight() <= text_block_->bottom();
    y += line->GetHeight();
  }
  return false;
}

Posn TextRenderer::MapPointToPosn(gfx::PointF pt) const {
  DCHECK(gfx_);
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
      auto lMap = cell->MapXToPosn(*gfx_, x);
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
  DCHECK(gfx_);
  auto const pFont = FontSet::Get(*gfx_, m_pBuffer->GetDefaultStyle())->
        FindFont(*gfx_, 'x');
  auto const height = AlignHeightToPixel(*gfx_, pFont->height());
  return static_cast<int>(text_block_->height() / height);
}

void TextRenderer::Prepare(const Selection& selection) {
  selection_ = selection;
}

void TextRenderer::Render() {
  DCHECK(gfx_);
  DCHECK(!text_block_->rect().empty());
  text_block_->EnsureLinePoints();
  screen_text_block_->Render(text_block_.get());

  // FIXME 2007-08-05 yosi@msn.com We should expose show/hide
  // ruler settings to both script and UI.

  // Ruler
  auto const pFont = FontSet::Get(*gfx_, m_pBuffer->GetDefaultStyle())->
    FindFont(*gfx_, 'x');

  // FIXME 2007-08-05 yosi@msn.com We should expose rule position to
  // user.
  auto const num_columns = 81;
  auto const width_of_M = AlignWidthToPixel(*gfx_, pFont->GetCharWidth('M'));
  drawVLine(*gfx_, gfx::Brush(*gfx_, gfx::ColorF::LightGray),
            text_block_->left() + width_of_M * num_columns,
            text_block_->top(), text_block_->bottom());
}

void TextRenderer::Reset() {
  screen_text_block_->Reset();
}

bool TextRenderer::ScrollDown() {
  DCHECK(gfx_);
  if (!GetStart())
    return false;
  auto const lGoal = GetStart() - 1;
  auto const lStart = m_pBuffer->ComputeStartOfLine(lGoal);
  TextFormatter formatter(*gfx_, text_block_.get(), selection_, lStart);
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

  return true;
}

bool TextRenderer::ScrollToPosn(Posn lPosn) {
  DCHECK(gfx_);
  if (isPosnVisible(lPosn))
    return false;

  auto const cLines = pageLines();
  auto const cLines2 = std::max(cLines / 2, 1);

  if (lPosn > GetStart()) {
    for (auto k = 0; k < cLines2; k++) {
        if (!ScrollUp())
          return k;
        if (isPosnVisible(lPosn))
          return true;
    }
  } else {
    for (int k = 0; k < cLines2; k++) {
      if (!ScrollDown())
        return k;
      if (isPosnVisible(lPosn))
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
    if (isPosnVisible(lPosn))
      break;
    if (!ScrollUp())
      break;
  }

  // If this page shows end of buffer, we shows lines as much as
  // posibble to fit in page.
  if (GetEnd() >= m_pBuffer->GetEnd()) {
    while (isPosnVisible(lPosn)) {
      if (!ScrollDown())
        return true;
    }
    ScrollUp();
  }
  return true;
}

bool TextRenderer::ScrollUp() {
  DCHECK(gfx_);
  text_block_->EnsureLinePoints();
  if (text_block_->IsShowEndOfDocument())
    return false;

  if (!text_block_->DiscardFirstLine())
    return false;

  text_block_->EnsureLinePoints();
  if (text_block_->IsShowEndOfDocument())
    return false;

  TextFormatter oFormatter(*gfx_, text_block_.get(), selection_,
                           text_block_->GetLast()->GetEnd());

  auto const line = oFormatter.FormatLine();
  text_block_->Append(line);
  return true;
}

void TextRenderer::SetGraphics(const gfx::Graphics* gfx) {
  gfx_ = gfx;
  screen_text_block_->SetGraphics(gfx);
}

void TextRenderer::SetBounds(const Rect& rect) {
  gfx::RectF rectf(rect);
  text_block_->SetBounds(rectf);
  screen_text_block_->SetBounds(rectf);
}

bool TextRenderer::ShouldFormat(const Selection& selection,
                                bool fSelection) const {
  if (text_block_->dirty())
    return true;

  // Buffer
  auto const lSelStart = selection.start;
  auto const lSelEnd = selection.end;

  // TextRenderer shows caret instead of seleciton.
  if (selection_.start == selection_.end) {
    if (lSelStart == lSelEnd) {
        #if DEBUG_DIRTY
            DEBUG_PRINTF("%p: clean with caret.\n", this);
        #endif // DEBUG_DIRTY
        return false;
    }

    if (!fSelection) {
        if (lSelEnd < GetStart() || lSelStart > GetEnd()) {
            #if DEBUG_DIRTY
                DEBUG_PRINTF("%p: clean with selection in outside.\n", this);
            #endif // DEBUG_DIRTY
            return false;
        }
    }

    #if DEBUG_DIRTY
        DEBUG_PRINTF("%p: Need to show selection.\n", this);
    #endif // DEBUG_DIRTY
    return true;
  }

  if (!fSelection) {
    // TextRenderer doesn't contain selection.
    if (selection_.end < GetStart() || selection_.start > GetEnd()) {
        if (lSelStart == lSelEnd) {
            #if DEBUG_DIRTY
                DEBUG_PRINTF("%p: clean with selection.\n", this);
            #endif // DEBUG_DIRTY
            return false;
        }

        if (lSelEnd < GetStart() || lSelStart > GetEnd())
            return false;
        #if DEBUG_DIRTY
            DEBUG_PRINTF("%p: Need to show selection.\n", this);
        #endif // DEBUG_DIRTY
        return true;
    }
  }

  // TextRenderer shows selection.
  if (selection_.start != lSelStart) {
    #if DEBUG_DIRTY
        DEBUG_PRINTF("%p: Selection start is changed.\n", this);
    #endif // DEBUG_DIRTY
    return true;
  }

  if (selection_.end != lSelEnd) {
    #if DEBUG_DIRTY
        DEBUG_PRINTF("%p: Selection end is changed.\n", this);
    #endif // DEBUG_DIRTY
    return true;
  }

  if (selection_.active != selection.active) {
    #if DEBUG_DIRTY
        DEBUG_PRINTF("%p: selection.active is changed.\n", this);
    #endif // DEBUG_DIRTY
    return true;
  }

  #if DEBUG_DIRTY
    DEBUG_PRINTF("%p is clean.\n", this);
  #endif // DEBUG_DIRTY

  return false;
}

bool TextRenderer::ShouldRender() const {
  return screen_text_block_->dirty();
}

}  // namespaec views
