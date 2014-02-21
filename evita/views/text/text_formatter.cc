// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/views/text/text_formatter.h"

#include "base/logging.h"
#include "evita/text/buffer.h"
#include "evita/text/interval.h"
#include "evita/views/text/render_cell.h"
#include "evita/views/text/render_text_block.h"
#include "evita/views/text/render_text_line.h"
#include "evita/views/text/text_renderer.h"

#define DEBUG_FORMAT 0

namespace views {
namespace rendering {

namespace {

const float cxLeftMargin = 7.0f;
const int k_nTabWidth = 4;

float AlignHeightToPixel(const gfx::Graphics& gfx, float height) {
  return gfx.AlignToPixel(gfx::SizeF(0.0f, height)).height;
}

float AlignWidthToPixel(const gfx::Graphics&, float width) {
  return width;
}

inline char16 toxdigit(int k) {
  if (k <= 9)
    return static_cast<char16>(k + '0');
 return static_cast<char16>(k - 10 + 'A');
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// EnumCI
//  Enumerator for characters and interval
//
class TextFormatter::EnumCI {
  private: Posn m_lBufEnd;
  private: Posn m_lBufStart;
  private: Posn m_lPosn;
  private: text::Buffer* m_pBuffer;
  private: text::Interval* m_pInterval;
  private: char16 m_rgwch[80];

  public: EnumCI(text::Buffer *pBuffer, Posn lPosn)
      : m_pBuffer(pBuffer),
        m_lPosn(lPosn) {
    m_pInterval = m_pBuffer->GetIntervalAt(m_lPosn);
    DCHECK(m_pInterval);
    fill();
  }

  public: bool AtEnd() const {
    return m_lBufStart == m_lBufEnd;
  }

  private: void fill() {
    auto const cwch = m_pBuffer->GetText(
        m_rgwch, m_lPosn, static_cast<Posn>(m_lPosn + lengthof(m_rgwch)));

    m_lBufStart = m_lPosn;
    m_lBufEnd   = m_lPosn + cwch;
  }

  public: char16 GetChar() const {
    if (AtEnd())
      return 0;
    DCHECK_GE(m_lPosn, m_lBufStart);
    DCHECK_LT(m_lPosn, m_lBufEnd);
    return m_rgwch[m_lPosn - m_lBufStart];
  }

  public: Posn GetPosn() const { return m_lPosn; }

  public: const StyleValues* GetStyle() const {
    if (AtEnd())
      return m_pBuffer->GetDefaultStyle();
    DCHECK(m_pInterval);
    return m_pInterval->GetStyle();
  }

  public: void Next() {
    if (AtEnd())
      return;
    m_lPosn += 1;
    if (m_lPosn >= m_lBufEnd)
      fill();

    if (m_lPosn >= m_pInterval->GetEnd()) {
      text::Interval* pNext = m_pInterval->GetNext();
      if (pNext)
        m_pInterval = pNext;
    }
  }
};

//////////////////////////////////////////////////////////////////////
//
// TextFormatter
//
TextFormatter::TextFormatter(const gfx::Graphics& gfx,
                             TextRenderer* pTextRenderer, Posn lStart)
    : m_gfx(gfx),
      m_pTextRenderer(pTextRenderer),
      m_oEnumCI(new EnumCI(pTextRenderer->GetBuffer(), lStart)) {
}

TextFormatter::~TextFormatter() {
}

void TextFormatter::Format() {
  #if DEBUG_FORMAT
    DEBUG_PRINTF("%p: start=%d " DEBUG_RECTF_FORMAT "\n",
        m_pTextRenderer, m_pTextRenderer->GetStart(),
        DEBUG_RECTF_ARG(m_pTextRenderer->m_oFormatBuf.rect()));
  #endif

  auto const cyTextRenderer = m_pTextRenderer->m_oFormatBuf.height();
  for (;;) {
    auto const pLine = new TextLine();

    bool fMore = FormatLine(pLine);
    DCHECK_GT(pLine->rect().height(), 0.0f);

    m_pTextRenderer->m_oFormatBuf.Append(pLine);

    // Line must have at least one cell other than filler.
    DCHECK_GE(pLine->GetEnd(), pLine->GetStart());

    if (m_pTextRenderer->m_oFormatBuf.GetHeight() >= cyTextRenderer) {
      // TextRenderer is filled up with lines.
      break;
    }

    if (!fMore) {
      // We have no more contents. Add a filler line.
      break;
    }
  }

  m_pTextRenderer->m_oFormatBuf.Finish();
}

// Returns true if more contents is avaialble, otherwise returns false.
bool TextFormatter::FormatLine(TextLine* pLine) {
  auto fMoreContents = true;
  pLine->set_start(m_oEnumCI->GetPosn());

  auto x = m_pTextRenderer->m_oFormatBuf.left();
  auto descent = 0.0f;
  auto ascent  = 0.0f;

  Cell* pCell;

  // Left margin
  {
    auto const cyMinHeight = 1.0f;

    pCell = new FillerCell(m_pTextRenderer->m_crBackground, cxLeftMargin,
                           cyMinHeight);
    pLine->AddCell(pCell);
    x += cxLeftMargin;
  }

  for (;;) {
    if (m_oEnumCI->AtEnd()) {
      pCell = formatMarker(MarkerCell::Kind_Eob);
      fMoreContents = false;
      break;
    }

    auto const wch = m_oEnumCI->GetChar();

    if (wch == 0x0A) {
      pCell = formatMarker(MarkerCell::Kind_Eol);
      m_oEnumCI->Next();
      break;
    }

    auto const cx = pCell->m_cx;

    pCell = formatChar(pCell, x, wch);
    if (!pCell) {
      pCell = formatMarker(MarkerCell::Kind_Wrap);
      break;
    }

    m_oEnumCI->Next();

    if (pLine->last_cell() == pCell) {
      x -= cx;
    } else {
      pLine->AddCell(pCell);
    }

    x += pCell->m_cx;
    descent = std::max(pCell->GetDescent(), descent);
    ascent  = std::max(pCell->GetHeight() - pCell->GetDescent(), ascent);
  }

  // We have at least one cell.
  //   o end of buffer: End-Of-Buffer MarkerCell
  //   o end of line:   End-Of-Line MarkerCell
  //   o wrapped line:  Warp MarkerCEll
  DCHECK(pCell);
  pLine->AddCell(pCell);

  x += pCell->m_cx;
  descent = std::max(pCell->GetDescent(), descent);
  ascent  = std::max(pCell->GetHeight() - pCell->GetDescent(), ascent);

  pLine->Fix(m_pTextRenderer->m_oFormatBuf.left(),
             m_pTextRenderer->m_oFormatBuf.top() + 
                 m_pTextRenderer->m_oFormatBuf.GetHeight(),
             ascent, descent);

  return fMoreContents;
}

//////////////////////////////////////////////////////////////////////
//
// TextFormatter::formatChar
//
Cell* TextFormatter::formatChar(
    Cell* pPrev,
    float x,
    char16 wch) {
  Color crColor;
  Color crBackground;
  TextDecoration  eDecoration;

  auto const lPosn = m_oEnumCI->GetPosn();
  const auto* const pStyle = m_oEnumCI->GetStyle();

  if (lPosn >= m_pTextRenderer->m_lSelStart &&
      lPosn < m_pTextRenderer->m_lSelEnd) {
    crColor      = m_pTextRenderer->m_crSelFg;
    crBackground = m_pTextRenderer->m_crSelBg;
    eDecoration  = TextDecoration_None;
  } else {
    crColor      = pStyle->GetColor();
    crBackground = pStyle->GetBackground();
    eDecoration  = pStyle->GetDecoration();
  }

  if (0x09 == wch) {
    auto const pFont = FontSet::Get(m_gfx, pStyle)->FindFont(m_gfx, 'x');
    auto const cxTab = AlignWidthToPixel(m_gfx, pFont->GetCharWidth(' ')) *
                          k_nTabWidth;
    auto const x2 = (x + cxTab - cxLeftMargin) / cxTab * cxTab;
    auto const cx = (x2 + cxLeftMargin) - x;
    auto const cxM = AlignWidthToPixel(m_gfx, pFont->GetCharWidth('M'));
    if (pPrev && x2 + cxM > m_pTextRenderer->m_oFormatBuf.right())
      return nullptr;

    return new MarkerCell(
        pStyle->GetMarker(),
        crBackground,
        cx,
        AlignHeightToPixel(m_gfx, pFont->height()),
        pFont->descent(),
        lPosn,
        MarkerCell::Kind_Tab);
  }

  auto const pFont = wch < 0x20 ? 
      nullptr :
      FontSet::Get(m_gfx, pStyle)->FindFont(m_gfx, wch);

  if (!pFont) {
    auto const pFont = FontSet::Get(m_gfx, pStyle)->FindFont(m_gfx, 'u');
    base::string16 string;

    if (wch < 0x20) {
      string.push_back('^');
      string.push_back(static_cast<base::char16>(wch + 0x40));
    } else {
      string.push_back('u');
      string.push_back(toxdigit((wch >> 12) & 15));
      string.push_back(toxdigit((wch >> 8) & 15));
      string.push_back(toxdigit((wch >> 4) & 15));
      string.push_back(toxdigit((wch >> 0) & 15));
    }

    auto const cxUni = 6.0f + AlignWidthToPixel(m_gfx,
                                                pFont->GetTextWidth(string));
    auto const cxM = AlignWidthToPixel(m_gfx, pFont->GetCharWidth('M'));
    if (pPrev && x + cxUni + cxM > m_pTextRenderer->m_oFormatBuf.right())
      return nullptr;

    return new UnicodeCell(m_gfx, pStyle, pStyle->GetMarker(), crBackground,
                           pFont, cxUni, lPosn, string);
  }

  auto const cx = AlignWidthToPixel(m_gfx, pFont->GetCharWidth(wch));

  if (pPrev) {
    auto const cxM = AlignWidthToPixel(m_gfx, pFont->GetCharWidth('M'));
    if (x + cx + cxM > m_pTextRenderer->m_oFormatBuf.right()) {
      // We doesn't have enough room for a char in the line.
      return nullptr;
    }

    if (pPrev->Merge(pFont, crColor, crBackground, eDecoration, cx)) {
      pPrev->as<TextCell>()->AddChar(wch);
      return pPrev;
    }
  }

  return new TextCell(m_gfx, pStyle, crColor, crBackground, pFont, cx, lPosn,
                      base::string16(1u, wch));
}

Cell* TextFormatter::formatMarker(MarkerCell::Kind  eKind) {
    Color crColor;
    Color crBackground;

    Posn lPosn = m_oEnumCI->GetPosn();
    const StyleValues* pStyle = m_oEnumCI->GetStyle();

    if (lPosn >= m_pTextRenderer->m_lSelStart &&
        lPosn < m_pTextRenderer->m_lSelEnd)
    {
        crColor      = m_pTextRenderer->m_crSelFg;
        crBackground = m_pTextRenderer->m_crSelBg;
    }
    else
    {
        crColor      = pStyle->GetMarker();
        crBackground = pStyle->GetBackground();
    }

    Font* pFont = FontSet::Get(m_gfx, pStyle)->FindFont(m_gfx, 'x');
    MarkerCell* pCell = new MarkerCell(
        crColor,
        crBackground,
        AlignWidthToPixel(m_gfx, pFont->GetCharWidth('x')),
        AlignHeightToPixel(m_gfx, pFont->height()),
        pFont->descent(),
        m_oEnumCI->GetPosn(),
        eKind);
    return pCell;
}

} // namespace rendering
} // namespace views
