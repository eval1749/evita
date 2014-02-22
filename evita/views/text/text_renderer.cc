// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#define DEBUG_DIRTY  0
#define DEBUG_DISPBUF 0
#define DEBUG_FORMAT 0
#define DEBUG_RENDER 0
#include "evita/views/text/text_renderer.h"

#include <algorithm>
#include <list>
#include <memory>
#include <utility>

#include "evita/gfx_base.h"
#include "evita/dom/buffer.h"
#include "evita/views/text/render_cell.h"
#include "evita/views/text/render_text_line.h"
#include "evita/views/text/text_formatter.h"

namespace views {
namespace {

float AlignHeightToPixel(const gfx::Graphics& gfx, float height) {
  return gfx.AlignToPixel(gfx::SizeF(0.0f, height)).height;
}

float AlignWidthToPixel(const gfx::Graphics&, float width) {
  return width;
}

inline gfx::ColorF ColorToColorF(Color color) {
  COLORREF const cr = color;
  return gfx::ColorF(
      static_cast<float>(GetRValue(cr)) / 255,
      static_cast<float>(GetGValue(cr)) / 255,
      static_cast<float>(GetBValue(cr)) / 255);
}

inline void drawLine(const gfx::Graphics& gfx, const gfx::Brush& brush,
                     float sx, float sy, float ex, float ey) {
  gfx.DrawLine(brush, sx, sy, ex, ey);
}

inline void drawVLine(const gfx::Graphics& gfx, const gfx::Brush& brush,
                      float x, float sy, float ey) {
  drawLine(gfx, brush, x, sy, x, ey);
}

inline void fillRect(const gfx::Graphics& gfx, const gfx::RectF& rect,
                     gfx::ColorF color) {
  gfx::Brush fill_brush(gfx, color);
  gfx.FillRectangle(fill_brush, rect);
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
      m_lStart(0),
      m_lEnd(0),
      m_crBackground(0) {
  m_pBuffer->AddObserver(this);
}

TextRenderer::~TextRenderer() {
  m_pBuffer->RemoveObserver(this);
}

void TextRenderer::fillBottom() const {
  DCHECK(gfx_);
  auto const lines_bottom = m_oFormatBuf.top() + m_oFormatBuf.GetHeight();
  if (lines_bottom < m_oFormatBuf.bottom()) {
    gfx::RectF rect(m_oFormatBuf.rect());
    rect.top = lines_bottom;
    #if DEBUG_RENDER
      DEBUG_PRINTF("fill rect #%06X " DEBUG_RECTF_FORMAT "\n",
          m_crBackground, DEBUG_RECTF_ARG(rect));
    #endif
    fillRect(*gfx_, rect, ColorToColorF(m_crBackground));
  }

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
            m_oFormatBuf.left() + width_of_M * num_columns,
            m_oFormatBuf.top(), m_oFormatBuf.bottom());
}

void TextRenderer::fillRight(const TextLine* pLine) const {
  DCHECK(gfx_);
  gfx::RectF rc;
  rc.left  = m_oFormatBuf.left() + pLine->GetWidth();
  rc.right = m_oFormatBuf.right();
  if (rc.left >= rc.right)
    return;
  rc.top = pLine->top();
  rc.bottom = ::ceilf(rc.top + pLine->GetHeight());
  fillRect(*gfx_, rc, ColorToColorF(m_crBackground));
}

TextLine* TextRenderer::FindLine(Posn lPosn) const {
  if (lPosn < m_lStart || lPosn > m_lEnd)
    return nullptr;

  for (auto const line : m_oFormatBuf.lines()) {
    if (lPosn < line->m_lEnd)
      return line;
  }

  // We must not be here.
  return nullptr;
}

void TextRenderer::Format(Posn lStart) {
  DCHECK(gfx_);
  m_oFormatBuf.Reset();
  m_lStart = lStart;

  TextFormatter oFormatter(*gfx_, &m_oFormatBuf, m_pBuffer, lStart, selection_);
  oFormatter.Format();
  m_lEnd = GetLastLine()->GetEnd();
}

TextLine* TextRenderer::FormatLine(Posn lStart) {
  DCHECK(gfx_);
  // TODO(yosi) Why do we need to reset |TextBlock|?
  m_oFormatBuf.Reset();
  TextFormatter oFormatter(*gfx_, &m_oFormatBuf, m_pBuffer, lStart, selection_);
  return oFormatter.FormatLine();
}

bool TextRenderer::isPosnVisible(Posn lPosn) const {
  if (lPosn < m_lStart)
    return false;
  if (lPosn >= m_lEnd)
    return false;

  auto y = m_oFormatBuf.top();
  for (const auto& line : m_oFormatBuf.lines()) {
    if (lPosn >= line->GetStart() && lPosn < line->GetEnd())
      return y + line->GetHeight() <= m_oFormatBuf.bottom();
    y += line->GetHeight();
  }
  return false;
}

Posn TextRenderer::MapPointToPosn(gfx::PointF pt) const {
  DCHECK(gfx_);
  if (pt.y < m_oFormatBuf.top())
    return GetStart();
  if (pt.y >= m_oFormatBuf.bottom())
    return GetEnd();

  auto yLine = m_oFormatBuf.top();
  for (const auto line : m_oFormatBuf.lines()) {
    auto const y = pt.y - yLine;
    yLine += line->GetHeight();

    if (y >= line->GetHeight())
      continue;

    auto xCell = m_oFormatBuf.left();
    if (pt.x < xCell)
      return line->GetStart();

    auto lPosn = line->GetEnd() - 1;
    for (const auto cell : line->cells()) {
      auto x = pt.x - xCell;
      xCell += cell->m_cx;
      auto lMap = cell->MapXToPosn(*gfx_, x);
      if (lMap >= 0)
        lPosn = lMap;
      if (x >= 0 && x < cell->m_cx)
        break;
    }
    return lPosn;
  }
  return GetEnd() - 1;
}

// Maps specified buffer position to window point and returns true. If
// specified buffer point isn't in window, this function returns false.
//
// A TextRenderer object must be formatted with the latest buffer.
//
gfx::RectF TextRenderer::MapPosnToPoint(Posn lPosn) const {
  DCHECK(gfx_);
  if (lPosn < m_lStart || lPosn > m_lEnd)
    return gfx::RectF();

  auto y = m_oFormatBuf.top();
  for (auto const line : m_oFormatBuf.lines()) {
    if (lPosn >= line->m_lStart && lPosn < line->m_lEnd) {
        auto x = m_oFormatBuf.left();
        for (const auto cell : line->cells()) {
          float cx = cell->MapPosnToX(*gfx_, lPosn);
          if (cx >= 0) {
            return gfx::RectF(gfx::PointF(x + cx, y),
                              gfx::SizeF(cell->m_cx, cell->m_cy));
          }
          x += cell->m_cx;
        }
    }
    y += line->GetHeight();
  }
  return gfx::RectF();
}

// Returns number of lines to be displayed in this page when using
// buffer's default style.
int TextRenderer::pageLines() const {
  DCHECK(gfx_);
  auto const pFont = FontSet::Get(*gfx_, m_pBuffer->GetDefaultStyle())->
        FindFont(*gfx_, 'x');
  auto const height = AlignHeightToPixel(*gfx_, pFont->height());
  return static_cast<int>(m_oFormatBuf.height() / height);
}

void TextRenderer::Prepare(const Selection& selection) {
  selection_ = selection;
  m_crBackground = m_pBuffer->GetDefaultStyle()->GetBackground();
}

namespace {

//////////////////////////////////////////////////////////////////////
//
// LineCopier
//
class LineCopier {
  private: typedef TextRenderer::TextBlock TextBlock;
  private: typedef TextLine Line;
  private: typedef std::list<Line*>::const_iterator LineIterator;

  private: const TextBlock* destination_;
  private: const gfx::Graphics& gfx_;
  private: const TextBlock* source_;
  private: const std::unique_ptr<gfx::Bitmap> screen_bitmap_;

  public: LineCopier(const gfx::Graphics& gfx,
                     const TextBlock* destination,
                     const TextBlock* source)
      : gfx_(gfx), destination_(destination), source_(source),
        screen_bitmap_(CreateBitmap(gfx, source)) {
  }

  private: static std::unique_ptr<gfx::Bitmap> CreateBitmap(
      const gfx::Graphics& gfx,
      const TextBlock* source) {
    if (source->dirty())
      return std::unique_ptr<gfx::Bitmap>();
    gfx::RectU screen_rect(gfx->GetPixelSize());
    auto bitmap = std::make_unique<gfx::Bitmap>(gfx);
    auto hr = (*bitmap)->CopyFromRenderTarget(nullptr, gfx, &screen_rect);
    if (FAILED(hr)) {
      DVLOG(0) << "CopyFromRenderTarget: hr=0x" << std::hex << hr;
      return std::unique_ptr<gfx::Bitmap>();
    }
    return std::move(bitmap);
  }

  private: void Copy(float dst_top, float dst_bottom, float src_top) const {
    auto const src_bottom = src_top + dst_bottom - dst_top;

    auto const height = std::min(
        std::min(destination_->rect().bottom, dst_bottom) - dst_top,
        std::min(destination_->rect().bottom, src_bottom) - src_top);

    auto const right = destination_->rect().right;
    gfx::RectF dst_rect(0.0f, dst_top, right, dst_top + height);
    gfx::RectF src_rect(0.0f, src_top, right, src_top + height);
    DCHECK_EQ(dst_rect.size(), src_rect.size());

    auto const opacity = 1.0f;
    gfx_->DrawBitmap(*screen_bitmap_, dst_rect, opacity,
                     D2D1_BITMAP_INTERPOLATION_MODE_LINEAR,
                     src_rect);

    #if DEBUG_RENDER
      if (dst_rect.height() > 8) {
          fillRect(gfx_,
                   gfx::RectF(gfx::PointF(dst_rect.left + 4, dst_rect.top + 2),
                              gfx::SizeF(4.0f, dst_rect.height() - 4)),
                   gfx::ColorF::LightGreen);
      }
      DEBUG_PRINTF("copy " DEBUG_RECTF_FORMAT " to " DEBUG_RECTF_FORMAT "\n",
          DEBUG_RECTF_ARG(src_rect), DEBUG_RECTF_ARG(dst_rect));
    #endif
  }

  private: LineIterator FindSameLine(const Line* line) const {
    auto const top = (*source_->lines().begin())->top();
    for (auto runner = source_->lines().begin();
         runner != source_->lines().end(); ++runner) {
      auto const candidate = *runner;
      if (candidate->bottom() - top > source_->height()) {
        // This line is rendered partially.
        break;
      }
      if (candidate->Equal(line))
        return runner;
    }
    return source_->lines().end();
  }

  public: LineIterator TryCopy(const LineIterator& new_start) const {
    if (!screen_bitmap_)
      return new_start;

    auto present_start = FindSameLine(*new_start);
    if (present_start == source_->lines().end())
      return new_start;

    auto const new_end = destination_->lines().end();
    auto const present_end = source_->lines().end();

    auto new_last = new_start;
    auto new_runner = new_start;
    ++new_runner;
    auto present_runner = present_start;
    ++present_runner;
    while (new_runner != new_end && present_runner != present_end) {
      if ((*present_runner)->bottom() > destination_->bottom() ||
          !(*new_runner)->Equal(*present_runner)) {
        break;
      }
      new_last = new_runner;
      ++new_runner;
      ++present_runner;
    }

    if ((*new_start)->top() != (*present_start)->top()) {
      Copy((*new_start)->top(), (*new_last)->bottom(),
           (*present_start)->top());
      ++new_last;
    }
    return new_last;
  }

  DISALLOW_COPY_AND_ASSIGN(LineCopier);
};

} // namespace

bool TextRenderer::Render() {
  DCHECK(gfx_);
  DCHECK(!m_oFormatBuf.rect().empty());
  DCHECK(!m_oScreenBuf.rect().empty());
  auto number_of_rendering = 0;
  m_oFormatBuf.EnsureLinePoints();
  auto const format_line_end = m_oFormatBuf.lines().end();
  auto format_line_runner = m_oFormatBuf.lines().begin();
  auto const screen_line_end = m_oScreenBuf.lines().end();
  auto screen_line_runner = m_oScreenBuf.lines().begin();
  while (format_line_runner != format_line_end &&
         screen_line_runner != screen_line_end) {
    if ((*format_line_runner)->rect() != (*screen_line_runner)->rect() ||
        !(*format_line_runner)->Equal(*screen_line_runner)) {
      break;
    }
    ++format_line_runner;
    ++screen_line_runner;
  }

  if (format_line_runner != format_line_end) {
    LineCopier line_copier(*gfx_, &m_oFormatBuf, &m_oScreenBuf);
    // Note: LineCopier uses ID2D1Bitmap::CopyFromRenderTarget. It should be
    // called without clipping.
    gfx::Graphics::AxisAlignedClipScope clip_scope(*gfx_, m_oFormatBuf.rect());
    while (format_line_runner != format_line_end) {
      format_line_runner = line_copier.TryCopy(format_line_runner);
      if (format_line_runner == format_line_end)
        break;
      auto const format_line = *format_line_runner;
      format_line->Render(*gfx_);
      fillRight(format_line);
      ++number_of_rendering;
      ++format_line_runner;
    }
  }

  fillBottom();

  // Update m_oScreenBuf for next rendering.
  m_oScreenBuf.Reset();
  for (const auto line : m_oFormatBuf.lines()) {
    m_oScreenBuf.Append(line->Copy());
  }

  m_oScreenBuf.Finish();

  #if DEBUG_RENDER
    if (number_of_rendering >= 1) {
      DEBUG_PRINTF("%p"
                   " redraw=%d"
                   " r[%d, %d] s[%d, %d]"
                   " screen=" DEBUG_RECTF_FORMAT "\n",
                   this,
                   number_of_rendering,
                   m_lStart, m_lEnd,
                   selection_.start, selection_.end,
                   DEBUG_RECTF_ARG(m_oScreenBuf.rect()));
    }
  #endif // DEBUG_RENDER
  return number_of_rendering > 0;
}

void TextRenderer::Reset() {
  m_oScreenBuf.Reset();
}

bool TextRenderer::ScrollDown() {
  DCHECK(gfx_);
  if (m_oFormatBuf.GetHeight() >= m_oFormatBuf.height() &&
      !m_oFormatBuf.ScrollDown()) {
    // This page shows only one line.
    return false;
  }

  auto const lGoal  = m_lStart - 1;
  auto const lStart = m_pBuffer->ComputeStartOfLine(lGoal);
  TextFormatter formatter(*gfx_, &m_oFormatBuf, m_pBuffer, lStart, selection_);

  for (;;) {
    auto const line = formatter.FormatLine();
    if (lGoal < line->GetEnd()) {
      m_oFormatBuf.Prepend(line);
      break;
    }
  }

  while (m_oFormatBuf.GetHeight() > m_oFormatBuf.height()) {
    if (!m_oFormatBuf.ScrollDown())
      break;
  }

  m_lStart = m_oFormatBuf.GetFirst()->GetStart();
  m_lEnd = m_oFormatBuf.GetLast()->GetEnd();
  return true;
}

bool TextRenderer::ScrollToPosn(Posn lPosn) {
  DCHECK(gfx_);
  if (isPosnVisible(lPosn))
    return false;

  auto const cLines = pageLines();
  auto const cLines2 = std::max(cLines / 2, 1);

  if (lPosn > m_lStart) {
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
  // Note: We should scroll up if page shows end of buffer. Since,
  // the last line may not be fully visible.

  // Recycle the first line.
  if (!m_oFormatBuf.ScrollUp()) {
    // This page shows only one line.
    return false;
  }

  TextFormatter oFormatter(*gfx_, &m_oFormatBuf, m_pBuffer,
                           GetLastLine()->GetEnd(), selection_);

  auto const line = oFormatter.FormatLine();
  m_oFormatBuf.Append(line);

  auto const cyTextRenderer = m_oFormatBuf.height();
  auto more = true;
  while (m_oFormatBuf.GetHeight() > cyTextRenderer) {
    if (!m_oFormatBuf.ScrollUp()) {
      more = false;
      break;
    }
  }

  m_lStart = m_oFormatBuf.GetFirst()->GetStart();
  m_lEnd = m_oFormatBuf.GetLast()->GetEnd();
  return more;
}

void TextRenderer::SetGraphics(const gfx::Graphics* gfx) {
  gfx_ = gfx;
}

void TextRenderer::SetRect(const Rect& rect) {
  gfx::RectF rectf(rect);
  m_oFormatBuf.SetRect(rectf);
  m_oScreenBuf.SetRect(rectf);
}

bool TextRenderer::ShouldFormat(const Selection& selection,
                                bool fSelection) const {
  if (m_oFormatBuf.dirty())
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
        if (lSelEnd < m_lStart || lSelStart > m_lEnd) {
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
    if (selection_.end < m_lStart || selection_.start > m_lEnd) {
        if (lSelStart == lSelEnd) {
            #if DEBUG_DIRTY
                DEBUG_PRINTF("%p: clean with selection.\n", this);
            #endif // DEBUG_DIRTY
            return false;
        }

        if (lSelEnd < m_lStart || lSelStart > m_lEnd)
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

  if (selection_.color != selection.color) {
    #if DEBUG_DIRTY
        DEBUG_PRINTF("%p: SelColor is changed.\n", this);
    #endif // DEBUG_DIRTY
    return true;
  }

  if (selection_.bgcolor != selection.bgcolor) {
    #if DEBUG_DIRTY
        DEBUG_PRINTF("%p: SelBackground is changed.\n", this);
    #endif // DEBUG_DIRTY
    return true;
  }

  #if DEBUG_DIRTY
    DEBUG_PRINTF("%p is clean.\n", this);
  #endif // DEBUG_DIRTY

  return false;
}

bool TextRenderer::ShouldRender() const {
  return m_oScreenBuf.dirty();
}

// text::BufferMutationObserver
void TextRenderer::DidDeleteAt(Posn offset, size_t) {
  m_oFormatBuf.SetBufferDirtyOffset(offset);
}

void TextRenderer::DidInsertAt(Posn offset, size_t) {
  m_oFormatBuf.SetBufferDirtyOffset(offset);
}

}  // namespaec views
