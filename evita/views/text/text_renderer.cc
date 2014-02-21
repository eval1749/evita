// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#define DEBUG_DIRTY  0
#define DEBUG_DISPBUF 0
#define DEBUG_FORMAT 0
#define DEBUG_RENDER 0
#include "evita/views/text/text_renderer.h"

#include <algorithm>
#include <utility>
#include <vector>

#include "base/strings/string16.h"
#include "evita/gfx_base.h"
#include "evita/dom/buffer.h"
#include "evita/views/text/render_cell.h"
#include "evita/views/text/render_text_line.h"
#include "evita/views/text/text_formatter.h"
#include "evita/vi_Selection.h"

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
    : m_pBuffer(buffer),
      m_lStart(0),
      m_lEnd(0),
      m_lSelStart(0),
      m_lSelEnd(0),
      m_crSelFg(0),
      m_crSelBg(0),
      m_crBackground(0) {
  m_pBuffer->AddObserver(this);
}

TextRenderer::~TextRenderer() {
  m_pBuffer->RemoveObserver(this);
}

void TextRenderer::fillBottom(const gfx::Graphics& gfx) const {
  auto const lines_bottom = m_oFormatBuf.top() + m_oFormatBuf.GetHeight();
  if (lines_bottom < m_oFormatBuf.bottom()) {
    gfx::RectF rect(m_oFormatBuf.rect());
    rect.top = lines_bottom;
    #if DEBUG_RENDER
      DEBUG_PRINTF("fill rect #%06X " DEBUG_RECTF_FORMAT "\n",
          m_crBackground, DEBUG_RECTF_ARG(rect));
    #endif
    fillRect(gfx, rect, ColorToColorF(m_crBackground));
  }

  // FIXME 2007-08-05 yosi@msn.com We should expose show/hide
  // ruler settings to both script and UI.

  // Ruler
  auto const pFont = FontSet::Get(gfx, m_pBuffer->GetDefaultStyle())->
    FindFont(gfx, 'x');

  // FIXME 2007-08-05 yosi@msn.com We should expose rule position to
  // user.
  auto const num_columns = 81;
  auto const width_of_M = AlignWidthToPixel(gfx, pFont->GetCharWidth('M'));
  drawVLine(gfx, gfx::Brush(gfx, gfx::ColorF::LightGray),
            m_oFormatBuf.left() + width_of_M * num_columns,
            m_oFormatBuf.top(), m_oFormatBuf.bottom());
}

void TextRenderer::fillRight(const gfx::Graphics& gfx,
                             const TextLine* pLine) const {
  gfx::RectF rc;
  rc.left  = m_oFormatBuf.left() + pLine->GetWidth();
  rc.right = m_oFormatBuf.right();
  if (rc.left >= rc.right)
    return;
  rc.top = pLine->top();
  rc.bottom = ::ceilf(rc.top + pLine->GetHeight());
  fillRect(gfx, rc, ColorToColorF(m_crBackground));
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

void TextRenderer::Format(const gfx::Graphics& gfx, gfx::RectF page_rect,
                  const Selection& selection, Posn lStart) {
  ASSERT(!page_rect.is_empty());
  Prepare(selection);
  formatAux(gfx, page_rect, lStart);
}

void TextRenderer::formatAux(const gfx::Graphics& gfx, gfx::RectF page_rect,
                     Posn lStart) {
  ASSERT(!page_rect.is_empty());
  m_oFormatBuf.Reset(page_rect);
  m_lStart = lStart;

  TextFormatter oFormatter(gfx, this, lStart);
  oFormatter.Format();
  m_lEnd = GetLastLine()->GetEnd();
}

TextLine* TextRenderer::FormatLine(const gfx::Graphics& gfx,
                             const gfx::RectF& page_rect,
                             const Selection& selection,
                             Posn lStart) {
  Prepare(selection);
  m_oFormatBuf.Reset(page_rect);

  TextFormatter oFormatter(gfx, this, lStart);
  auto const line = new TextLine();
  oFormatter.FormatLine(line);
  return line;
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

Posn TextRenderer::MapPointToPosn(const gfx::Graphics& gfx,
                                  gfx::PointF pt) const {
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
      auto lMap = cell->MapXToPosn(gfx, x);
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
gfx::RectF TextRenderer::MapPosnToPoint(const gfx::Graphics& gfx,
                                        Posn lPosn) const {
  if (lPosn < m_lStart || lPosn > m_lEnd)
    return gfx::RectF();

  auto y = m_oFormatBuf.top();
  for (auto const line : m_oFormatBuf.lines()) {
    if (lPosn >= line->m_lStart && lPosn < line->m_lEnd) {
        auto x = m_oFormatBuf.left();
        for (const auto cell : line->cells()) {
          float cx = cell->MapPosnToX(gfx, lPosn);
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
int TextRenderer::pageLines(const gfx::Graphics& gfx) const {
  auto const pFont = FontSet::Get(gfx, m_pBuffer->GetDefaultStyle())->
        FindFont(gfx, 'x');
  auto const height = AlignHeightToPixel(gfx, pFont->height());
  return static_cast<int>(m_oFormatBuf.height() / height);
}

void TextRenderer::Prepare(const Selection& selection) {
  auto& buffer = *selection.GetBuffer();

  // Selection
  m_lSelStart = selection.GetStart();
  m_lSelEnd = selection.GetEnd();
  m_crSelFg = selection.GetColor();
  m_crSelBg = selection.GetBackground();

  #if DEBUG_FORMAT
    DEBUG_PRINTF("selection: %d...%d 0x%x/0x%x\n",
        m_lSelStart, m_lSelEnd,
        m_crSelFg, m_crSelBg);
  #endif // DEBUG_FORMAT

  // TextRenderer
  m_crBackground = buffer.GetDefaultStyle()->GetBackground();
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
        screen_bitmap_(CreateBitmap(gfx, destination->rect(), source->rect())) {
  }

  private: static std::unique_ptr<gfx::Bitmap> CreateBitmap(
      const gfx::Graphics& gfx,
      const gfx::RectF& rect, const gfx::RectF& src_rect) {
    if (rect.width() > src_rect.width())
      return std::unique_ptr<gfx::Bitmap>();
    gfx::RectU screen_rect(gfx->GetPixelSize());
    if (static_cast<float>(screen_rect.width()) < src_rect.width())
      return std::unique_ptr<gfx::Bitmap>();

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
    for (auto runner = source_->lines().begin();
         runner != source_->lines().end(); ++runner) {
      if ((*runner)->Equal(line))
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

bool TextRenderer::Render(const gfx::Graphics& gfx) {
  ASSERT(!m_oFormatBuf.rect().is_empty());
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
    LineCopier line_copier(gfx, &m_oFormatBuf, &m_oScreenBuf);
    // Note: LineCopier uses ID2D1Bitmap::CopyFromRenderTarget. It should be
    // called without clipping.
    gfx::Graphics::AxisAlignedClipScope clip_scope(gfx, m_oFormatBuf.rect());
    while (format_line_runner != format_line_end) {
      format_line_runner = line_copier.TryCopy(format_line_runner);
      if (format_line_runner == format_line_end)
        break;
      auto const format_line = *format_line_runner;
      format_line->Render(gfx);
      fillRight(gfx, format_line);
      ++number_of_rendering;
      ++format_line_runner;
    }
  }

  fillBottom(gfx);

  // Update m_oScreenBuf for next rendering.
  m_oScreenBuf.Reset(m_oFormatBuf.rect());
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
                   m_lSelStart, m_lSelEnd,
                   DEBUG_RECTF_ARG(m_oScreenBuf.rect()));
    }
  #endif // DEBUG_RENDER
  return number_of_rendering > 0;
}

void TextRenderer::Reset() {
  m_oScreenBuf.Reset(gfx::RectF());
}

bool TextRenderer::ScrollDown(const gfx::Graphics& gfx) {
  if (!m_lStart) {
    // This page shows start of buffer.
    return false;
  }

  auto const pLine = m_oFormatBuf.GetHeight() < m_oFormatBuf.height() ?
      new TextLine() : m_oFormatBuf.ScrollDown();
  if (!pLine) {
    // This page shows only one line.
    return false;
  }

  auto const lGoal  = m_lStart - 1;
  auto const lStart = m_pBuffer->ComputeStartOfLine(lGoal);
  TextFormatter formatter(gfx, this, lStart);

  do {
    pLine->Reset();
    formatter.FormatLine(pLine);
  } while (lGoal >= pLine->GetEnd());

  m_oFormatBuf.Prepend(pLine);

  while (m_oFormatBuf.GetHeight() > m_oFormatBuf.height()) {
    auto const pLast = m_oFormatBuf.ScrollDown();
    if (!pLast)
      break;
    delete pLast;
  }

  m_lStart = GetFirstLine()->GetStart();
  m_lEnd   = GetLastLine()->GetEnd();
  return true;
}

bool TextRenderer::ScrollToPosn(const gfx::Graphics& gfx, Posn lPosn) {
  if (isPosnVisible(lPosn))
    return false;

  auto const cLines = pageLines(gfx);
  auto const cLines2 = std::max(cLines / 2, 1);

  if (lPosn > m_lStart) {
    for (auto k = 0; k < cLines2; k++) {
        if (!ScrollUp(gfx))
          return k;
        if (isPosnVisible(lPosn))
          return true;
    }
  } else {
    for (int k = 0; k < cLines2; k++) {
      if (!ScrollDown(gfx))
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

  formatAux(gfx, m_oFormatBuf.rect(), lStart);
  for (;;) {
    if (isPosnVisible(lPosn))
      break;
    if (!ScrollUp(gfx))
      break;
  }

  // If this page shows end of buffer, we shows lines as much as
  // posibble to fit in page.
  if (GetEnd() >= m_pBuffer->GetEnd()) {
    while (isPosnVisible(lPosn)) {
      if (!ScrollDown(gfx))
        return true;
    }
    ScrollUp(gfx);
  }
  return true;
}

bool TextRenderer::ScrollUp(const gfx::Graphics& gfx) {
  // Note: We should scroll up if page shows end of buffer. Since,
  // the last line may not be fully visible.

  // Recycle the first line.
  auto const pLine = m_oFormatBuf.ScrollUp();
  if (!pLine) {
    // This page shows only one line.
    return false;
  }

  pLine->Reset();

  TextFormatter oFormatter(gfx, this, GetLastLine()->GetEnd());

  auto const fMore = oFormatter.FormatLine(pLine);
  m_oFormatBuf.Append(pLine);

  auto const cyTextRenderer = m_oFormatBuf.height();
  while (m_oFormatBuf.GetHeight() > cyTextRenderer) {
    auto const pFirst = m_oFormatBuf.ScrollUp();
    if (!pFirst)
      break;
    delete pFirst;
  }

  m_lStart = GetFirstLine()->GetStart();
  m_lEnd   = GetLastLine()->GetEnd();

  return fMore;
}

bool TextRenderer::ShouldFormat(const Rect& rc, const Selection& selection,
                      bool fSelection) const {
  if (m_oFormatBuf.dirty())
    return true;

  if (rc.width() != m_oFormatBuf.width()) {
      #if DEBUG_DIRTY
          DEBUG_PRINTF("%p: Width is changed.\n", this);
      #endif // DEBUG_DIRTY
      return true;
  }

  if (rc.height() != m_oFormatBuf.height()) {
      #if DEBUG_DIRTY
          DEBUG_PRINTF("%p: Height is changed.\n", this);
      #endif // DEBUG_DIRTY
      return true;
  }

  // Buffer
  auto const lSelStart = selection.GetStart();
  auto const lSelEnd = selection.GetEnd();

  // TextRenderer shows caret instead of seleciton.
  if (m_lSelStart == m_lSelEnd) {
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
    if (m_lSelEnd < m_lStart || m_lSelStart > m_lEnd) {
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
  if (m_lSelStart != lSelStart) {
    #if DEBUG_DIRTY
        DEBUG_PRINTF("%p: Selection start is changed.\n", this);
    #endif // DEBUG_DIRTY
    return true;
  }

  if (m_lSelEnd != lSelEnd) {
    #if DEBUG_DIRTY
        DEBUG_PRINTF("%p: Selection end is changed.\n", this);
    #endif // DEBUG_DIRTY
    return true;
  }

  if (m_crSelFg != selection.GetColor()) {
    #if DEBUG_DIRTY
        DEBUG_PRINTF("%p: SelColor is changed.\n", this);
    #endif // DEBUG_DIRTY
    return true;
  }

  if (m_crSelBg  != selection.GetBackground()) {
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
