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
#include "evita/text/interval.h"
#include "evita/views/text/render_cell.h"
#include "evita/vi_Selection.h"

namespace views {

namespace rendering {

const float cxLeftMargin = 7.0f;
const int k_nTabWidth = 4;
const float k_cyMinScroll = 100.0f;

inline char16 toxdigit(int k) {
  if (k <= 9)
    return static_cast<char16>(k + '0');
 return static_cast<char16>(k - 10 + 'A');
}

float AlignHeightToPixel(const gfx::Graphics& gfx, float height) {
  return gfx.AlignToPixel(gfx::SizeF(0.0f, height)).height;
}

float AlignWidthToPixel(const gfx::Graphics&, float width) {
  return width;
}

float FloorWidthToPixel(const gfx::Graphics& gfx, float width) {
  return gfx.FloorToPixel(gfx::SizeF(width, 0.0f)).width;
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

inline void drawHLine(const gfx::Graphics& gfx, const gfx::Brush& brush,
                      float sx, float ex, float y) {
  drawLine(gfx, brush, sx, y, ex, y);
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

static void DrawText(const gfx::Graphics& gfx, const Font& font,
              const gfx::Brush& text_brush, const gfx::RectF& rect,
              const base::string16& string) {
  font.DrawText(gfx, text_brush, rect, string);
  gfx.Flush();
}

//////////////////////////////////////////////////////////////////////
//
// FillerCell
//
class FillerCell final : public Cell {
  DECLARE_CASTABLE_CLASS(FillerCell, Cell);

  public: FillerCell(Color cr, float cx, float cy)
      : Cell(cr, cx, cy) {
  }

  public: FillerCell(const FillerCell& other);
  public: virtual ~FillerCell();

  public: virtual Cell* Copy() const override {
    return new FillerCell(*this);
  }

};

FillerCell::FillerCell(const FillerCell& other)
    : Cell(other) {
}

FillerCell::~FillerCell() {
}

//////////////////////////////////////////////////////////////////////
//
// MarkerCell
//
class MarkerCell final : public Cell {
  DECLARE_CASTABLE_CLASS(MarkerCell, Cell);

  public: enum Kind {
    Kind_Eob,
    Kind_Eol,
    Kind_Tab,
    Kind_Wrap,
  };

  private: Posn m_lStart;
  private: Posn m_lEnd;
  private: Color m_crColor;
  private: float m_iAscent;
  private: float m_iDescent;
  private: Kind  m_eKind;

  public: MarkerCell(Color crColor,
                     Color crBackground,
                     float cx,
                     float iHeight,
                     float iDescent,
                     Posn lPosn,
                     Kind eKind)
      : m_crColor(crColor),
        m_iAscent(iHeight - iDescent),
        m_iDescent(iDescent),
        m_eKind(eKind),
        m_lStart(lPosn),
        m_lEnd(eKind == Kind_Wrap ? lPosn : lPosn + 1),
        Cell(crBackground, cx, iHeight) {
  }

  public: MarkerCell(const MarkerCell& other);
  public: ~MarkerCell();

  public: virtual Cell* Copy() const {
    return new MarkerCell(*this);
  }

  public: virtual bool Equal(const Cell* pCell) const {
    if (!Cell::Equal(pCell))
      return false;
    // reinterpret_cast used between related classes: 'class1' and 'class2'
    #pragma warning(suppress: 4946)
    auto const pMarker = reinterpret_cast<const MarkerCell*>(pCell);
    if (!m_crColor.Equal(pMarker->m_crColor))
      return false;
    if (m_eKind != pMarker->m_eKind)
      return false;
    return true;
  }

  public: virtual Posn Fix(float iHeight, float iDescent) override {
    m_cy = iHeight;
    m_iDescent = iDescent;
    return m_lEnd;
  }

  public: virtual float GetDescent() const override { return m_iDescent; }

  public: virtual uint Hash() const override {
    auto nHash = Cell::Hash();
    nHash <<= 8;
    nHash ^= m_crColor.Hash();
    nHash <<= 8;
    nHash ^= m_eKind;
    return nHash;
  }

  public: virtual float MapPosnToX(const gfx::Graphics&,
                                   Posn lPosn) const override {
    if (lPosn < m_lStart)
      return -1.0f;
    if (lPosn >= m_lEnd)
      return -1.0f;
    return 0.0f;
  }

  public: virtual Posn MapXToPosn(const gfx::Graphics&,
                                  float) const override {
    return m_lStart;
  }

  public: virtual void Render(const gfx::Graphics& gfx,
                              const gfx::RectF& rect) const override {
    FillBackground(gfx, rect);

    auto const yBottom = rect.bottom - m_iDescent;
    auto const yTop    = yBottom - m_iAscent;
    auto const xLeft   = rect.left;
    auto const xRight  = rect.right;

    gfx::Brush stroke_brush(gfx, ColorToColorF(m_crColor));

    switch (m_eKind) {
      case Kind_Eob: { // Draw <-
        // FIXME 2007-06-13 We should get internal leading from font.
        auto const iInternalLeading = 3;
        auto const w = std::max(m_iAscent / 6, 2.0f);
        auto const y = yBottom - (m_iAscent - iInternalLeading) / 2;
        drawHLine(gfx, stroke_brush, xLeft, xRight, y);
        drawLine(gfx, stroke_brush, xLeft + w, y - w, xLeft, y);
        drawLine(gfx, stroke_brush, xLeft + w, y + w, xLeft, y);
        break;
      }

      case Kind_Eol: { // Draw V
        auto const y = yBottom - m_iAscent * 3 / 5;
        auto const w = std::max(m_cx / 6, 2.0f);
        auto const x = xLeft + m_cx / 2;
        drawVLine(gfx, stroke_brush, x, y, yBottom);
        drawLine(gfx, stroke_brush, x - w, yBottom - w, x, yBottom);
        drawLine(gfx, stroke_brush, x + w, yBottom - w, x, yBottom);
        break;
      }

      case Kind_Tab: { // Draw |_|
        auto const w = std::max(m_iAscent / 6, 2.0f);
        drawHLine(gfx, stroke_brush, xLeft + 2, xRight - 3, yBottom);
        drawVLine(gfx, stroke_brush, xLeft + 2, yBottom, yBottom - w * 2);
        drawVLine(gfx, stroke_brush, xRight - 3, yBottom, yBottom - w * 2);
        break;
      }

      case Kind_Wrap: { // Draw ->
        auto const ex = xRight - 1;
        auto const w = std::max(m_iAscent / 6, 2.0f);
        auto const y = yTop + m_iAscent / 2;
        drawHLine(gfx, stroke_brush, xLeft, ex, y);
        drawLine(gfx, stroke_brush, ex - w, y - w, xRight, y);
        drawLine(gfx, stroke_brush, ex - w, y + w, xRight, y);
        break;
      }

        default:
        CAN_NOT_HAPPEN();
    }
  }
};

MarkerCell::MarkerCell(const MarkerCell& other)
    : Cell(other),
      m_lStart(other.m_lStart),
      m_lEnd(other.m_lEnd),
      m_crColor(other.m_crColor),
      m_iAscent(other.m_iAscent),
      m_iDescent(other.m_iDescent),
      m_eKind(other.m_eKind) {
}

MarkerCell::~MarkerCell() {
}

//////////////////////////////////////////////////////////////////////
//
// TextCell
//
class TextCell : public Cell {
  DECLARE_CASTABLE_CLASS(TextCell, Cell);

  protected: Color            m_crColor;
  protected: TextDecoration   m_eDecoration;
  protected: float m_iDescent;

  protected: Posn             m_lStart;
  protected: Posn             m_lEnd;

  protected: Font*            m_pFont;
  private: base::string16 characters_;

  public: TextCell(
      const gfx::Graphics& gfx,
      const StyleValues* pStyle,
      Color           crColor,
      Color           crBackground,
      Font*           pFont,
      float           cx,
      Posn            lPosn,
      const base::string16& characters)
        : Cell(crBackground, cx,
               AlignHeightToPixel(gfx, pFont->height())),
          m_crColor(crColor),
          m_eDecoration(pStyle->GetDecoration()),
          m_lStart(lPosn),
          m_lEnd(lPosn + 1),
          m_pFont(pFont),
          characters_(characters) {
  }

  public: TextCell(const TextCell& other);
  public: virtual ~TextCell();

  public: const base::string16 characters() const { return characters_; }

  public: void AddChar(base::char16 char_code);

  private: virtual Cell* Copy() const override;

  // Equal - Returns true if specified cell is equal to this cell.
  public: virtual bool Equal(const Cell* pCell) const {
    if (!Cell::Equal(pCell))
      return false;
    // reinterpret_cast used between related classes: 'class1' and 'class2'
    #pragma warning(suppress: 4946)
    auto const pText = reinterpret_cast<const TextCell*>(pCell);
    if (!m_crColor.Equal(pText->m_crColor))
      return false;
    if (m_eDecoration != pText->m_eDecoration)
      return false;
    return characters_ == pText->characters_;
  }

  public: virtual Posn Fix(float iHeight, float iDescent) override {
      ASSERT(m_lStart <= m_lEnd);
      m_cy       = iHeight;
      m_iDescent = iDescent;
      return m_lEnd;
  }

  public: virtual float GetDescent() const override {
    return m_pFont->descent();
  }

  public: virtual uint Hash() const override final {
    uint nHash = Cell::Hash();
    nHash ^= m_crColor.Hash();
    nHash ^= m_pFont->Hash();
    nHash ^= m_eDecoration;
    nHash ^= static_cast<uint32_t>(characters_.length());
    for (auto ch : characters_) {
      nHash <<= 5;
      nHash ^= ch;
      nHash >>= 3;
    }
    return nHash;
  }

  public: virtual float MapPosnToX(const gfx::Graphics& gfx,
                                   Posn lPosn) const override final {
    if (lPosn < m_lStart)
      return -1;
    if (lPosn >= m_lEnd)
      return -1;
    auto const cwch = static_cast<size_t>(lPosn - m_lStart);
    if (!cwch)
      return 0;
    auto const width = m_pFont->GetTextWidth(characters_.data(), cwch);
    return FloorWidthToPixel(gfx, width);
  }

  public: virtual Posn MapXToPosn(const gfx::Graphics& gfx,
                                  float x) const override final {
    if (x >= m_cx)
      return m_lEnd;
    for (auto k = 1u; k <= characters_.length(); ++k) {
      auto const cx = FloorWidthToPixel(gfx,
        m_pFont->GetTextWidth(characters_.data(), k));
      if (x < cx)
        return static_cast<Posn>(m_lStart + k - 1);
    }
    return m_lEnd;
  }

  // Merge - Returns true if specified cell is merged to this cell.
  public: virtual bool Merge(
      Font*           pFont,
      Color           crColor,
      Color           crBackground,
      TextDecoration  eDecoration,
      float cx) override {
    if (m_pFont        != pFont) return false;
    if (m_crColor      != crColor) return false;
    if (m_crBackground != crBackground) return false;
    if (m_eDecoration  != eDecoration)  return false;
    m_cx   += cx;
    m_lEnd += 1;
    return true;
  }

  // Render - Render text of this cell
  public: virtual void Render(const gfx::Graphics& gfx,
                              const gfx::RectF& rect) const override {
    DCHECK(!characters_.empty());
    FillBackground(gfx, rect);
    gfx::Brush text_brush(gfx, ColorToColorF(m_crColor));
    DrawText(gfx, *m_pFont, text_brush, rect, characters_);

    auto const y = rect.bottom - m_iDescent -
                   (m_eDecoration != TextDecoration_None ? 1 : 0);
    #if SUPPORT_IME
    switch (m_eDecoration) {
      case TextDecoration_ImeInput:
        // TODO: We should use dotted line. It was PS_DOT.
        drawHLine(gfx, text_brush, rect.left, rect.right - 4, y + 3);
        break;

      case TextDecoration_ImeInactiveA:
        drawHLine(gfx, text_brush, rect.left, rect.right - 4, y + 3);
        break;

      case TextDecoration_ImeInactiveB:
        drawHLine(gfx, text_brush, rect.left, rect.right - 4, y + 3);
        break;

      case TextDecoration_ImeActive:
        drawHLine(gfx, text_brush, rect.left, rect.right - 4, y + 3);
        drawHLine(gfx, text_brush, rect.left, rect.right - 4, y + 2);
        break;

      case TextDecoration_None:
        break;

      case TextDecoration_GreenWave:
        // TODO: Implement TextDecoration_RedWave
        break;

      case TextDecoration_RedWave:
        // TODO: Implement TextDecoration_RedWave
        break;

      case TextDecoration_Underline:
        // TODO: Implement TextDecoration_Underline
        break;
    }
    #endif
  }
};

TextCell::TextCell(const TextCell& other)
    : Cell(other),
      m_crColor(other.m_crColor),
      m_eDecoration(other.m_eDecoration),
      m_iDescent(other.m_iDescent),
      m_lStart(other.m_lStart),
      m_lEnd(other.m_lEnd),
      m_pFont(other.m_pFont),
      characters_(other.characters_) {
}

TextCell::~TextCell() {
}

void TextCell::AddChar(base::char16 char_code) {
  characters_.push_back(char_code);
}

Cell* TextCell::Copy() const {
  return new TextCell(*this);
}

//////////////////////////////////////////////////////////////////////
//
// UnicodeCell
//
class UnicodeCell final : public TextCell {
  DECLARE_CASTABLE_CLASS(UnicodeCell, TextCell);

  public: UnicodeCell(
      const gfx::Graphics& gfx,
      const StyleValues*    pStyle,
      Color           crColor,
      Color           crBackground,
      Font*           pFont,
      float           cx,
      Posn            lPosn,
      const base::string16& characters)
        : TextCell(
              gfx,
              pStyle,
              crColor,
              crBackground,
              pFont,
              cx,
              lPosn,
              characters) {
      m_cy += 4;
  }

  public: UnicodeCell(const UnicodeCell& other);
  public: virtual ~UnicodeCell();

  private: virtual Cell* Copy() const override;

  public: virtual void Render(const gfx::Graphics& gfx,
                              const gfx::RectF& rect) const override {
    FillBackground(gfx, rect);

    gfx::Brush text_brush(gfx, ColorToColorF(m_crColor));
    DrawText(gfx, *m_pFont, text_brush, rect, characters());

    gfx.DrawRectangle(text_brush,
                      gfx::RectF(rect.left, rect.top,
                                 rect.right - 1, rect.bottom - 1));
  }
};

UnicodeCell::UnicodeCell(const UnicodeCell& other)
    : TextCell(other) {
}

UnicodeCell::~UnicodeCell() {
}

Cell* UnicodeCell::Copy() const {
  return new UnicodeCell(*this);
}

//////////////////////////////////////////////////////////////////////
//
// EnumCI
//  Enumerator for characters and interval
//
class EnumCI {
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
    ASSERT(m_pInterval);
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
    ASSERT(m_lPosn >= m_lBufStart);
    ASSERT(m_lPosn < m_lBufEnd);
    return m_rgwch[m_lPosn - m_lBufStart];
  }

  public: Posn GetPosn() const { return m_lPosn; }

  public: const StyleValues* GetStyle() const {
    if (AtEnd())
      return m_pBuffer->GetDefaultStyle();
    ASSERT(m_pInterval);
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
// Formatter
//
class Formatter {
  private: const gfx::Graphics& m_gfx;
  private: TextRenderer* const m_pTextRenderer;
  private: EnumCI m_oEnumCI;

  public: Formatter(const gfx::Graphics& gfx,
                    TextRenderer* pTextRenderer,
                    Posn lStart)
      : m_gfx(gfx),
        m_pTextRenderer(pTextRenderer),
        m_oEnumCI(pTextRenderer->GetBuffer(), lStart) {
  }

  public: ~Formatter();

  public: void Format();
  public: bool FormatLine(TextRenderer::TextLine*);

  private: Cell* formatChar(Cell*, float x, char16);
  private: Cell* formatMarker(MarkerCell::Kind);
  private: Cell* formatTab(int);

  DISALLOW_COPY_AND_ASSIGN(Formatter);
};

Formatter::~Formatter() {
}

void Formatter::Format() {
  #if DEBUG_FORMAT
    DEBUG_PRINTF("%p: start=%d " DEBUG_RECTF_FORMAT "\n",
        m_pTextRenderer, m_pTextRenderer->GetStart(),
        DEBUG_RECTF_ARG(m_pTextRenderer->m_oFormatBuf.rect()));
  #endif

  auto const cyTextRenderer = m_pTextRenderer->m_oFormatBuf.height();
  for (;;) {
    auto const pLine = new TextRenderer::TextLine();

    bool fMore = FormatLine(pLine);
    DCHECK_GT(pLine->rect().height(), 0.0f);

    m_pTextRenderer->m_oFormatBuf.Append(pLine);

    // Line must have at least one cell other than filler.
    ASSERT(pLine->GetEnd() >= pLine->GetStart());

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
bool Formatter::FormatLine(TextRenderer::TextLine* pLine) {
  auto fMoreContents = true;
  pLine->m_lStart = m_oEnumCI.GetPosn();

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
    if (m_oEnumCI.AtEnd()) {
      pCell = formatMarker(MarkerCell::Kind_Eob);
      fMoreContents = false;
      break;
    }

    auto const wch = m_oEnumCI.GetChar();

    if (wch == 0x0A) {
      pCell = formatMarker(MarkerCell::Kind_Eol);
      m_oEnumCI.Next();
      break;
    }

    auto const cx = pCell->m_cx;

    pCell = formatChar(pCell, x, wch);
    if (!pCell) {
      pCell = formatMarker(MarkerCell::Kind_Wrap);
      break;
    }

    m_oEnumCI.Next();

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
  ASSERT(pCell);
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
// Formatter::formatChar
//
Cell* Formatter::formatChar(
    Cell* pPrev,
    float x,
    char16 wch) {
  Color crColor;
  Color crBackground;
  TextDecoration  eDecoration;

  auto const lPosn = m_oEnumCI.GetPosn();
  const auto* const pStyle = m_oEnumCI.GetStyle();

  if (lPosn >= m_pTextRenderer->m_lSelStart && lPosn < m_pTextRenderer->m_lSelEnd) {
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

Cell* Formatter::formatMarker(MarkerCell::Kind  eKind) {
    Color crColor;
    Color crBackground;

    Posn lPosn = m_oEnumCI.GetPosn();
    const StyleValues* pStyle = m_oEnumCI.GetStyle();

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
        m_oEnumCI.GetPosn(),
        eKind);
    return pCell;
}

} // namespace rendering

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

TextRenderer::TextLine* TextRenderer::FindLine(Posn lPosn) const {
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

  Formatter oFormatter(gfx, this, lStart);
  oFormatter.Format();
  m_lEnd = GetLastLine()->GetEnd();
}

TextRenderer::TextLine* TextRenderer::FormatLine(const gfx::Graphics& gfx,
                             const gfx::RectF& page_rect,
                             const Selection& selection,
                             Posn lStart) {
  Prepare(selection);
  m_oFormatBuf.Reset(page_rect);

  Formatter oFormatter(gfx, this, lStart);
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
  private: typedef TextRenderer::DisplayBuffer DisplayBuffer;
  private: typedef TextRenderer::TextLine Line;
  private: typedef std::list<Line*>::const_iterator LineIterator;

  private: const DisplayBuffer* destination_;
  private: const gfx::Graphics& gfx_;
  private: const DisplayBuffer* source_;
  private: const std::unique_ptr<gfx::Bitmap> screen_bitmap_;

  public: LineCopier(const gfx::Graphics& gfx,
                     const DisplayBuffer* destination,
                     const DisplayBuffer* source)
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
  Formatter formatter(gfx, this, lStart);

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

  Formatter oFormatter(gfx, this, GetLastLine()->GetEnd());

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

//////////////////////////////////////////////////////////////////////
//
// TextRenderer::DisplayBuffer
//
TextRenderer::DisplayBuffer::DisplayBuffer()
    : dirty_(true),
      dirty_line_point_(true),
      m_cy(0) {
}

TextRenderer::DisplayBuffer::~DisplayBuffer() {
}

void TextRenderer::DisplayBuffer::Append(Line* pLine) {
  DCHECK_LT(pLine->GetHeight(), 100.0f);
  if (!dirty_line_point_) {
    auto const last_line = lines_.back();
    pLine->set_left_top(gfx::PointF(last_line->left(), last_line->bottom()));
  }
  lines_.push_back(pLine);
  m_cy += pLine->GetHeight();
}

void TextRenderer::DisplayBuffer::EnsureLinePoints() {
  if (!dirty_line_point_)
    return;
  auto line_top = top();
  for (auto line : lines_) {
    line->set_left_top(gfx::PointF(left(), line_top));
    line_top = line->bottom();
  }

  dirty_line_point_ = false;
}

void TextRenderer::DisplayBuffer::Finish() {
  dirty_ = lines_.empty();
  dirty_line_point_ = dirty_;
}

void TextRenderer::DisplayBuffer::Prepend(Line* line) {
  DCHECK_LT(line->GetHeight(), 100.0f);
  lines_.push_front(line);
  m_cy += line->GetHeight();
  dirty_line_point_ = true;
}

void TextRenderer::DisplayBuffer::Reset(const gfx::RectF& new_rect) {
  #if DEBUG_DISPBUF
    DEBUG_PRINTF("%p " DEBUG_RECTF_FORMAT " to " DEBUG_RECTF_FORMAT "\n",
        this, DEBUG_RECTF_ARG(rect()), DEBUG_RECTF_ARG(new_rect));
  #endif

  for (auto const line : lines_) {
    delete line;
  }

  lines_.clear();
  dirty_ = true;
  dirty_line_point_ = true;
  m_cy = 0;
  rect_ = new_rect;
}

TextRenderer::TextLine* TextRenderer::DisplayBuffer::ScrollDown() {
  if (lines_.empty())
    return nullptr;

  auto const line = lines_.back();
  lines_.pop_back();
  m_cy -= line->GetHeight();
  return line;
}

TextRenderer::TextLine* TextRenderer::DisplayBuffer::ScrollUp() {
  if (lines_.empty())
    return nullptr;

  auto const line = lines_.front();
  lines_.pop_front();
  m_cy -= line->GetHeight();
  dirty_line_point_ = true;
  return line;
}

void TextRenderer::DisplayBuffer::SetBufferDirtyOffset(Posn offset) {
  if (dirty_)
    return;
  dirty_ = GetFirst()->GetStart() >= offset ||
           GetLast()->GetEnd() >= offset;
}

//////////////////////////////////////////////////////////////////////
//
// Line
//
TextRenderer::TextLine::TextLine(const Line& other)
    : m_nHash(other.m_nHash),
      m_lEnd(other.m_lEnd),
      m_lStart(other.m_lStart),
      rect_(other.rect_) {
  for (auto cell : other.cells_) {
    cells_.push_back(cell->Copy());
  }
}

TextRenderer::TextLine::TextLine()
    : m_nHash(0),
      m_lStart(0),
      m_lEnd(0) {
}

TextRenderer::TextLine::~TextLine() {
}

void TextRenderer::TextLine::set_left_top(const gfx::PointF& left_top) {
  rect_.right = rect_.width() + left_top.x;
  rect_.bottom = rect_.height() + left_top.y;
  rect_.left = left_top.x;
  rect_.top = left_top.y;
}

TextRenderer::TextLine* TextRenderer::TextLine::Copy() const {
  return new TextLine(*this);
}

bool TextRenderer::TextLine::Equal(const Line* other) const {
  if (Hash() != other->Hash())
    return false;
  if (cells_.size() != other->cells_.size())
    return false;
  auto other_it = other->cells_.begin();
  for (auto cell : cells_) {
    if (!cell->Equal(*other_it))
      return false;
    ++other_it;
  }
  return true;
}

void TextRenderer::TextLine::AddCell(Cell* cell) {
  cells_.push_back(cell);
}

void TextRenderer::TextLine::Fix(float left, float top,
                                 float ascent, float descent) {
  auto const height = ascent + descent;
  DCHECK_LT(height, 100.0f);
  auto right = left;
  for (auto cell : cells_) {
    auto const lEnd = cell->Fix(height, descent);
    if (lEnd >= 0)
      m_lEnd = lEnd;
    right += cell->GetWidth();
  }
  rect_.left = left;
  rect_.top = top;
  rect_.right = right;
  rect_.bottom = top + height;
}

uint TextRenderer::TextLine::Hash() const {
  if (m_nHash)
    return m_nHash;
  for (const auto cell : cells_) {
    m_nHash <<= 5;
    m_nHash ^= cell->Hash();
    m_nHash >>= 3;
  }
  return m_nHash;
}

Posn TextRenderer::TextLine::MapXToPosn(const gfx::Graphics& gfx,
                                        float xGoal) const {
  auto xCell = 0.0f;
  auto lPosn = GetEnd() - 1;
  for (const auto cell : cells_) {
    auto const x = xGoal - xCell;
    xCell += cell->m_cx;
    auto const lMap = cell->MapXToPosn(gfx, x);
    if (lMap >= 0)
      lPosn = lMap;
    if (x >= 0 && x < cell->m_cx)
      break;
  }
  return lPosn;
}

void TextRenderer::TextLine::Render(const gfx::Graphics& gfx) const {
  auto x = rect_.left;
  for (auto cell : cells_) {
    gfx::RectF rect(x, rect_.top, x + cell->m_cx,
                    ::ceilf(rect_.top + cell->m_cy));
    cell->Render(gfx, rect);
    x = rect.right;
  }
  gfx.Flush();
}

void TextRenderer::TextLine::Reset() {
  for (auto cell : cells_) {
    delete cell;
  }
  cells_.clear();
  rect_ = gfx::RectF();
  m_nHash = 0;
  m_lStart = -1;
  m_lEnd = -1;
}

}  // namespaec views
