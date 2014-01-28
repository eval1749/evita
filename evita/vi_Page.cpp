#include "precomp.h"
//////////////////////////////////////////////////////////////////////////////
//
// evcl - listener - Page
// listener/winapp/vi_Page.cpp
//
// Copyright (C) 1996-2007 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evcl3/mainline/listener/winapp/vi_Page.cpp#3 $
//
#define DEBUG_DIRTY  0
#define DEBUG_DISPBUF 0
#define DEBUG_FORMAT 0
#define DEBUG_HEAP   0
#define DEBUG_RENDER 0
#include "evita/vi_Page.h"

#include "evita/ed_interval.h"
#include "evita/ed_util.h"
#include "evita/gfx_base.h"
#include "evita/dom/buffer.h"
#include "evita/vi_Selection.h"
#include "evita/vi_util.h"
#include <algorithm>
#include <utility>
#include <vector>

namespace PageInternal {

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
              const char16* chars, uint num_chars) {
  font.DrawText(gfx, text_brush, rect, chars, num_chars);
  gfx.Flush();
}

enum CellKind {
  CellKind_Filler,
  CellKind_Marker,
  CellKind_Text,
  CellKind_Unicode,
};

//////////////////////////////////////////////////////////////////////
//
// Cell
//
class Cell
    : public DoubleLinkedNode_<Cell, Page::Line>,
      public ObjectInHeap {
  public: float m_cx;
  public: float m_cy;

  protected: Color m_crBackground;

  public: Cell(Color cr, float cx, float cy)
      : m_crBackground(cr),
        m_cx(cx),
        m_cy(cy) {
    ASSERT(cx >= 1);
    ASSERT(cy >= 1);
  }

  public: explicit Cell(const Cell& other)
      : m_crBackground(other.m_crBackground),
        m_cx(other.m_cx),
        m_cy(other.m_cy) {
  }

  public: virtual Cell* Copy(HANDLE, char16*) const = 0;

  public: virtual bool Equal(const Cell* pCell) const {
    if (pCell->GetKind() != GetKind()) return false;
    if (pCell->m_cx != m_cx) return false;
    if (pCell->m_cy != m_cy) return false;
    if (!pCell->m_crBackground.Equal(m_crBackground)) return false;
    return true;
  }

  protected: void FillBackground(const gfx::Graphics& gfx,
                                 const gfx::RectF& rect) const {
    fillRect(gfx, gfx::RectF(rect.left, rect.top, ::ceilf(rect.right),
                             ::ceilf(rect.bottom)),
             ColorToColorF(m_crBackground));
  }

  public: virtual Posn Fix(const char16*, float iHeight, float) {
    m_cy = iHeight;
    return -1;
  }

  public: virtual float GetDescent() const { return 0; }
  public: float GetHeight() const { return m_cy; }
  public: virtual CellKind GetKind() const = 0;
  public: float GetWidth() const { return m_cx; }

  public: virtual uint Hash() const {
    uint nHash = static_cast<uint>(m_cx);
    nHash ^= static_cast<uint>(m_cy);
    nHash ^= m_crBackground.Hash();
    return nHash;
  }

  public: virtual float MapPosnToX(const gfx::Graphics&, Posn) const {
    return -1.0f;
  }

    // MapXToPosn - x is cell relative.
  public: virtual Posn MapXToPosn(const gfx::Graphics&, float) const {
    return -1;
  }

  public: virtual bool Merge(Font*, Color, Color, TextDecoration, float) {
    return false;
  }

  public: virtual void Render(const gfx::Graphics& gfx,
                               const gfx::RectF& rect) const {
    FillBackground(gfx, rect);
  }
};

//////////////////////////////////////////////////////////////////////
//
// FillerCell
//
class FillerCell final : public Cell {
  public: FillerCell(Color cr, float cx, float cy)
      : Cell(cr, cx, cy) {
  }

  public: virtual Cell* Copy(HANDLE hHeap, char16*) const override {
    return new(hHeap) FillerCell(*this);
  }

  public: virtual CellKind GetKind() const override {
    return CellKind_Filler;
  }
};

//////////////////////////////////////////////////////////////////////
//
// MarkerCell
//
class MarkerCell final : public Cell {
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

  public: virtual Cell* Copy(HANDLE hHeap, char16*) const {
    return new(hHeap) MarkerCell(*this);
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

  public: virtual Posn Fix(const char16*, float iHeight,
                           float iDescent) override {
    m_cy = iHeight;
    m_iDescent = iDescent;
    return m_lEnd;
  }

  public: virtual float GetDescent() const override { return m_iDescent; }
  public: virtual CellKind GetKind() const override {
    return CellKind_Marker;
  }

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

//////////////////////////////////////////////////////////////////////
//
// TextCell
//
class TextCell : public Cell {
  protected: Color            m_crColor;
  protected: TextDecoration   m_eDecoration;
  protected: float m_iDescent;

  protected: Posn             m_lStart;
  protected: Posn             m_lEnd;

  protected: Font*            m_pFont;
  protected: const char16*    m_pwch;
  protected: uint             m_cwch;
  protected: uint             m_ofs;

  public: TextCell(
      const gfx::Graphics& gfx,
      const StyleValues* pStyle,
      Color           crColor,
      Color           crBackground,
      Font*           pFont,
      float           cx,
      Posn            lPosn,
      uint            ofs,
      uint            cwch = 1)
        : m_crColor(crColor),
          m_eDecoration(pStyle->GetDecoration()),
          m_lStart(lPosn),
          m_lEnd(lPosn + 1),
          m_pFont(pFont),
          m_pwch(nullptr),
          m_cwch(cwch),
          m_ofs(ofs),
          Cell(crBackground, cx,
               AlignHeightToPixel(gfx, pFont->height())) {
  }

  public: virtual Cell* Copy(HANDLE hHeap, char16* pwch) const {
    ASSERT(pwch);
    auto const pCell = new(hHeap) TextCell(*this);
    pCell->m_pwch = pwch + m_ofs;
    return pCell;
  }

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
    if (m_cwch != pText->m_cwch)
      return false;
    auto const cb = sizeof(char16) * m_cwch;
    return !::memcmp(m_pwch, pText->m_pwch, cb);
  }

  public: virtual Posn Fix(const char16* pwch, float iHeight,
                           float iDescent) override {
      ASSERT(m_lStart <= m_lEnd);
      m_pwch     = pwch + m_ofs;
      m_cy       = iHeight;
      m_iDescent = iDescent;
      return m_lEnd;
  }

  public: virtual float GetDescent() const override {
    return m_pFont->descent();
  }

  public: virtual CellKind GetKind() const override {
    return CellKind_Text;
  }

  public: virtual uint Hash() const override final {
    uint nHash = Cell::Hash();
    nHash ^= m_crColor.Hash();
    nHash ^= m_pFont->Hash();
    nHash ^= m_eDecoration;
    nHash ^= m_cwch;

    auto const pEnd = m_pwch + m_cwch;
    for (const char16* p = m_pwch; p < pEnd; p++) {
      nHash <<= 5; nHash ^= *p; nHash >>= 3;
    }
    return nHash;
  }

  public: virtual float MapPosnToX(const gfx::Graphics& gfx,
                                   Posn lPosn) const override final {
    if (lPosn < m_lStart)
      return -1;
    if (lPosn >= m_lEnd)
      return -1;
    auto const cwch = lPosn - m_lStart;
    if (!cwch)
      return 0;
    return FloorWidthToPixel(
        gfx, 
        m_pFont->GetTextWidth(m_pwch, static_cast<uint>(cwch)));
  }

  public: virtual Posn MapXToPosn(const gfx::Graphics& gfx,
                                  float x) const override final {
    if (x >= m_cx)
      return m_lEnd;
    for (uint k = 1; k <= m_cwch; ++k) {
      auto const cx = FloorWidthToPixel(gfx, m_pFont->GetTextWidth(m_pwch, k));
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
    m_cwch += 1;
    m_lEnd += 1;
    return true;
  }

  // Render - Render text of this cell
  public: virtual void Render(const gfx::Graphics& gfx,
                              const gfx::RectF& rect) const override {
    ASSERT(m_cwch);
    ASSERT(m_pwch);
    FillBackground(gfx, rect);
    gfx::Brush text_brush(gfx, ColorToColorF(m_crColor));
    DrawText(gfx, *m_pFont, text_brush, rect, m_pwch, m_cwch);

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

//////////////////////////////////////////////////////////////////////
//
// UnicodeCell
//
class UnicodeCell final : public TextCell {
  public: UnicodeCell(
      const gfx::Graphics& gfx,
      const StyleValues*    pStyle,
      Color           crColor,
      Color           crBackground,
      Font*           pFont,
      float           cx,
      Posn            lPosn,
      uint            ofs,
      uint            cwch) :
          TextCell(
              gfx,
              pStyle,
              crColor,
              crBackground,
              pFont,
              cx,
              lPosn,
              ofs,
              cwch) {
      m_cy += 4;
  }

  public: virtual Cell* Copy(HANDLE hHeap, char16* pwch) const {
    ASSERT(pwch);
    auto const pCell = new(hHeap) UnicodeCell(*this);
    pCell->m_pwch = pwch + m_ofs;
    return pCell;
  }

  public: virtual CellKind GetKind() const {
    return CellKind_Unicode;
  }

  public: virtual void Render(const gfx::Graphics& gfx,
                              const gfx::RectF& rect) const override {
    FillBackground(gfx, rect);

    gfx::Brush text_brush(gfx, ColorToColorF(m_crColor));
    DrawText(gfx, *m_pFont, text_brush, rect, m_pwch, m_cwch);

    gfx.DrawRectangle(text_brush,
                      gfx::RectF(rect.left, rect.top,
                                 rect.right - 1, rect.bottom - 1));
  }
};

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
  private: HANDLE const m_hObjHeap;
  private: Page*  const m_pPage;
  private: LocalCharSink_<> m_oCharSink;
  private: EnumCI m_oEnumCI;

  public: Formatter(const gfx::Graphics& gfx,
                    HANDLE hHeap,
                    Page* pPage,
                    Posn lStart)
      : m_gfx(gfx),
        m_hObjHeap(hHeap),
        m_pPage(pPage),
        m_oCharSink(hHeap),
        m_oEnumCI(pPage->GetBuffer(), lStart) {
  }

  public: void Format();
  public: bool FormatLine(Page::Line*);

  private: Cell* formatChar(Cell*, float x, char16);
  private: Cell* formatMarker(MarkerCell::Kind);
  private: Cell* formatTab(int);

  DISALLOW_COPY_AND_ASSIGN(Formatter);
};

void Formatter::Format() {
  #if DEBUG_FORMAT
    DEBUG_PRINTF("%p: start=%d " DEBUG_RECTF_FORMAT "\n",
        m_pPage, m_pPage->GetStart(), 
        DEBUG_RECTF_ARG(m_pPage->m_oFormatBuf.rect()));
  #endif

  auto const cyPage = m_pPage->m_oFormatBuf.height();
  for (;;) {
    Page::Line* pLine = m_pPage->m_oFormatBuf.NewLine();

    bool fMore = FormatLine(pLine);
    ASSERT(pLine->m_iHeight >= 1);

    m_pPage->m_oFormatBuf.Append(pLine);

    // Line must have at least one cell other than filler.
    ASSERT(pLine->GetEnd() >= pLine->GetStart());

    if (m_pPage->m_oFormatBuf.GetHeight() >= cyPage) {
      // Page is filled up with lines.
      break;
    }

    if (!fMore) {
      // We have no more contents. Add a filler line.
      break;
    }
  }
}

// Returns true if more contents is avaialble, otherwise returns false.
bool Formatter::FormatLine(Page::Line* pLine) {
  auto fMoreContents = true;
  pLine->m_lStart = m_oEnumCI.GetPosn();
  m_oCharSink.Reset();

  auto x = m_pPage->m_oFormatBuf.left();
  auto iDescent = 0.0f;
  auto iAscent  = 0.0f;

  Cell* pCell;

  // Left margin
  {
    auto const cyMinHeight = 1.0f;

    pCell = new(m_hObjHeap) FillerCell(m_pPage->m_crBackground,
                                       cxLeftMargin, cyMinHeight);
    pLine->cells().Append(pCell);
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

    if (pLine->cells().GetLast() == pCell) {
      x -= cx;
    } else {
      pLine->cells().Append(pCell);
    }

    x += pCell->m_cx;
    iDescent = std::max(pCell->GetDescent(), iDescent);
    iAscent  = std::max(pCell->GetHeight() - pCell->GetDescent(),  iAscent);
  }

  // We have at least one cell.
  //   o end of buffer: End-Of-Buffer MarkerCell
  //   o end of line:   End-Of-Line MarkerCell
  //   o wrapped line:  Warp MarkerCEll
  ASSERT(pCell);
  pLine->cells().Append(pCell);

  x += pCell->m_cx;
  iDescent = std::max(pCell->GetDescent(), iDescent);
  iAscent  = std::max(pCell->GetHeight() - pCell->GetDescent(),  iAscent);

  pLine->m_iHeight = iAscent + iDescent;

  pLine->m_cwch = m_oCharSink.GetLength();
  pLine->m_pwch = m_oCharSink.Fix();
  ASSERT(!pLine->m_cwch || pLine->m_pwch);
  pLine->Fix(iDescent);

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

  if (lPosn >= m_pPage->m_lSelStart && lPosn < m_pPage->m_lSelEnd) {
    crColor      = m_pPage->m_crSelFg;
    crBackground = m_pPage->m_crSelBg;
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
    if (pPrev && x2 + cxM > m_pPage->m_oFormatBuf.right())
      return nullptr;

    return new(m_hObjHeap) MarkerCell(
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
    char16 rgwch[5];
    size_t cwch;

    if (wch < 0x20) {
        rgwch[0] = '^';
        rgwch[1] = static_cast<char16>(wch + 0x40);
        cwch = 2;
    } else {
        rgwch[0] = 'u';
        rgwch[1] = toxdigit((wch >> 12) & 15);
        rgwch[2] = toxdigit((wch >> 8) & 15);
        rgwch[3] = toxdigit((wch >> 4) & 15);
        rgwch[4] = toxdigit((wch >> 0) & 15);
        cwch = 5;
    }

    auto const cxUni = 6.0f +
        AlignWidthToPixel(m_gfx, pFont->GetTextWidth(rgwch, cwch));
    auto const cxM = AlignWidthToPixel(m_gfx, pFont->GetCharWidth('M'));
    if (pPrev && x + cxUni + cxM > m_pPage->m_oFormatBuf.right())
      return nullptr;

    UnicodeCell* pCell = new(m_hObjHeap) UnicodeCell(
        m_gfx,
        pStyle,
        pStyle->GetMarker(),
        crBackground,
        pFont,
        cxUni,
        lPosn,
        m_oCharSink.GetLength(),
        cwch);

    m_oCharSink.Add(rgwch, cwch);
    return pCell;
  }

  auto const cx = AlignWidthToPixel(m_gfx, pFont->GetCharWidth(wch));

  if (!pPrev) {
      // We must draw a char at start of window line.
      auto const pCell = new(m_hObjHeap) TextCell(m_gfx, pStyle, crColor,
                                                  crBackground, pFont, cx,
                                                  lPosn,
                                                  m_oCharSink.GetLength());
      m_oCharSink.Add(wch);
      return pCell;
  }

  auto const cxM = AlignWidthToPixel(m_gfx, pFont->GetCharWidth('M'));
  if (x + cx + cxM > m_pPage->m_oFormatBuf.right()) {
    // We doesn't have enough room for a char in the line.
    return nullptr;
  }

  if (pPrev->Merge(pFont, crColor, crBackground, eDecoration, cx)) {
    m_oCharSink.Add(wch);
    return pPrev;
  }

  {
    auto const pCell = new(m_hObjHeap) TextCell(m_gfx, pStyle, crColor,
                                                crBackground, pFont, cx,
                                                lPosn,
                                                m_oCharSink.GetLength());
    m_oCharSink.Add(wch);
    return pCell;
  }
}

Cell* Formatter::formatMarker(MarkerCell::Kind  eKind) {
    Color crColor;
    Color crBackground;

    Posn lPosn = m_oEnumCI.GetPosn();
    const StyleValues* pStyle = m_oEnumCI.GetStyle();

    if (lPosn >= m_pPage->m_lSelStart &&
        lPosn < m_pPage->m_lSelEnd)
    {
        crColor      = m_pPage->m_crSelFg;
        crBackground = m_pPage->m_crSelBg;
    }
    else
    {
        crColor      = pStyle->GetMarker();
        crBackground = pStyle->GetBackground();
    }

    Font* pFont = FontSet::Get(m_gfx, pStyle)->FindFont(m_gfx, 'x');
    MarkerCell* pCell = new(m_hObjHeap) MarkerCell(
        crColor,
        crBackground,
        AlignWidthToPixel(m_gfx, pFont->GetCharWidth('x')),
        AlignHeightToPixel(m_gfx, pFont->height()),
        pFont->descent(),
        m_oEnumCI.GetPosn(),
        eKind);
    return pCell;
}

} // namespace PageInternal

using namespace PageInternal;

void Page::fillBottom(const gfx::Graphics& gfx, float y) const {
  if (y < m_oFormatBuf.bottom()) {
    gfx::RectF rect(m_oFormatBuf.left(), y, m_oFormatBuf.right(),
                    m_oFormatBuf.bottom());
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
  auto const num_columns = 80;
  auto const width_of_M = AlignWidthToPixel(gfx, pFont->GetCharWidth('M'));
  drawVLine(gfx, gfx::Brush(gfx, gfx::ColorF::LightGray),
            m_oFormatBuf.left() + width_of_M * num_columns,
            m_oFormatBuf.top(), m_oFormatBuf.bottom());
}

void Page::fillRight(const gfx::Graphics& gfx, const Line* pLine,
                     float y) const {
  gfx::RectF rc;
  rc.left  = m_oFormatBuf.left() + pLine->GetWidth();
  rc.right = m_oFormatBuf.right();
  if (rc.left < rc.right) {
    rc.top = y;
    rc.bottom = ::ceilf(y + pLine->GetHeight());
    fillRect(gfx, rc, ColorToColorF(m_crBackground));
  }
}

Page::Line* Page::FindLine(Posn lPosn) const {
  if (lPosn < m_lStart || lPosn > m_lEnd)
    return nullptr;

  for (auto& line : m_oFormatBuf.lines()) {
    if (lPosn < line.m_lEnd)
      return const_cast<Line*>(&line);
  }

  // We must not be here.
  return nullptr;
}

void Page::Format(const gfx::Graphics& gfx, gfx::RectF page_rect,
                  const Selection& selection, Posn lStart) {
  ASSERT(!page_rect.is_empty());
  Prepare(selection);
  formatAux(gfx, page_rect, lStart);
}

void Page::formatAux(const gfx::Graphics& gfx, gfx::RectF page_rect,
                     Posn lStart) {
  ASSERT(!page_rect.is_empty());
  m_oFormatBuf.Reset(page_rect);
  m_lStart = lStart;

  Formatter oFormatter(gfx, m_oFormatBuf.GetHeap(), this, lStart);
  oFormatter.Format();
  m_lEnd = GetLastLine()->GetEnd();
}

Page::Line* Page::FormatLine(const gfx::Graphics& gfx,
                             const gfx::RectF& page_rect,
                             const Selection& selection,
                             Posn lStart) {
  Prepare(selection);
  m_oFormatBuf.Reset(page_rect);

  auto const hHeap = m_oFormatBuf.GetHeap();
  Formatter oFormatter(gfx, hHeap, this, lStart);

  auto& line = *m_oFormatBuf.NewLine();
  oFormatter.FormatLine(&line);
  return &line;
}

//////////////////////////////////////////////////////////////////////
//
// Page
//
Page::Page()
    : m_pBuffer(nullptr),
      m_lStart(0),
      m_lEnd(0),
      m_nModfTick(-1),
      m_lSelStart(0),
      m_lSelEnd(0),
      m_crSelFg(0),
      m_crSelBg(0),
      m_crBackground(0) {
}

Page::~Page() {
}

bool Page::IsDirty(const Rect& rc, const Selection& selection,
                   bool fSelection) const {
  if (!m_pBuffer) {
      #if DEBUG_DIRTY
          DEBUG_PRINTF("%p: No buffer.\n", this);
      #endif // DEBUG_DIRTY
      return true;
  }

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
  const auto* const pBuffer = selection.GetBuffer();
  if (m_pBuffer != pBuffer) {
    #if DEBUG_DIRTY
        DEBUG_PRINTF("%p: Buffer is changed.\n", this);
    #endif // DEBUG_DIRTY
    return true;
  }

  if (m_nModfTick != pBuffer->GetModfTick()) {
    #if DEBUG_DIRTY
        DEBUG_PRINTF("%p: ModfTick is changed.\n", this);
    #endif // DEBUG_DIRTY
    return true;
  }

  auto const lSelStart = selection.GetStart();
  auto const lSelEnd = selection.GetEnd();

  // Page shows caret instead of seleciton.
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
    // Page doesn't contain selection.
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

  // Page shows selection.
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

bool Page::isPosnVisible(Posn lPosn) const {
  if (lPosn < m_lStart)
    return false;
  if (lPosn >= m_lEnd)
    return false;

  auto y = m_oFormatBuf.top();
  for (const auto& line : m_oFormatBuf.lines()) {
    if (lPosn >= line.GetStart() && lPosn < line.GetEnd())
      return y + line.GetHeight() <= m_oFormatBuf.bottom();
    y += line.GetHeight();
  }
  return false;
}

Posn Page::MapPointToPosn(const gfx::Graphics& gfx, gfx::PointF pt) const {
  if (pt.y < m_oFormatBuf.top())
    return GetStart();
  if (pt.y >= m_oFormatBuf.bottom())
    return GetEnd();

  auto yLine = m_oFormatBuf.top();
  for (const auto& line : m_oFormatBuf.lines()) {
    auto const y = pt.y - yLine;
    yLine += line.GetHeight();

    if (y >= line.GetHeight())
      continue;

    auto xCell = m_oFormatBuf.left();
    if (pt.x < xCell)
      return line.GetStart();

    auto lPosn = line.GetEnd() - 1;
    for (const auto& cell : line.cells()) {
      auto x = pt.x - xCell;
      xCell += cell.m_cx;
      auto lMap = cell.MapXToPosn(gfx, x);
      if (lMap >= 0)
        lPosn = lMap;
      if (x >= 0 && x < cell.m_cx)
        break;
    }
    return lPosn;
  }
  return GetEnd() - 1;
}

// Maps specified buffer position to window point and returns true. If
// specified buffer point isn't in window, this function returns false.
//
// A Page object must be formatted with the latest buffer.
//
gfx::RectF Page::MapPosnToPoint(const gfx::Graphics& gfx, Posn lPosn) const {
  if (lPosn < m_lStart || lPosn > m_lEnd)
    return gfx::RectF();

  auto y = m_oFormatBuf.top();
  for (const auto& line : m_oFormatBuf.lines()) {
    if (lPosn >= line.m_lStart && lPosn < line.m_lEnd) {
        auto x = m_oFormatBuf.left();
        for (const auto& cell : line.cells()) {
          float cx = cell.MapPosnToX(gfx, lPosn);
          if (cx >= 0) {
            return gfx::RectF(gfx::PointF(x + cx, y),
                              gfx::SizeF(cell.m_cx, cell.m_cy));
          }
          x += cell.m_cx;
        }
    }
    y += line.GetHeight();
  }
  return gfx::RectF();
}

// Returns number of lines to be displayed in this page when using
// buffer's default style.
int Page::pageLines(const gfx::Graphics& gfx) const {
  auto const pFont = FontSet::Get(gfx, m_pBuffer->GetDefaultStyle())->
        FindFont(gfx, 'x');
  auto const height = AlignHeightToPixel(gfx, pFont->height());
  return static_cast<int>(m_oFormatBuf.height() / height);
}

void Page::Prepare(const Selection& selection) {
  auto& buffer = *selection.GetBuffer();
  m_pBuffer = &buffer;
  m_nModfTick = buffer.GetModfTick();

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

  // Page
  m_crBackground = buffer.GetDefaultStyle()->GetBackground();
}

namespace {

//////////////////////////////////////////////////////////////////////
//
// LineWithTop
//
class LineWithTop {
  private: typedef Page::Line Line;

  private: const Line* line_;
  private: float line_top_;

  public: LineWithTop(const Line* line, float line_top)
      : line_(line), line_top_(line_top) {
  }

  public: LineWithTop()
      : line_(nullptr), line_top_(0.0f) {
  }

  public: operator bool() const { return line_; }
  public: bool operator!() const { return !line_; }
  public: operator const Line*() const { return line_; }

  public: const Line& operator*() const {
    ASSERT(line_);
    return *line_;
  }

  public: const Line* operator->() const {
    ASSERT(line_);
    return line_;
  }

  public: LineWithTop& operator++() {
    ASSERT(line_);
    line_top_ = line_bottom();
    line_ = line_->GetNext();
    return *this;
  }

  public: bool operator==(const LineWithTop& other) const {
    ASSERT(line_);
    ASSERT(!!other);
    return line_top_ == other.line_top_ && line_->Equal(other.line_);
  }

  public: bool operator!=(const LineWithTop& other) const {
    ASSERT(line_);
    ASSERT(!!other);
    return line_top_ != other.line_top_ || !line_->Equal(other.line_);
  }

  public: float height() const {
    ASSERT(line_);
    return line_->GetHeight();
  }

  public: float line_bottom() const {
    ASSERT(line_);
    return line_top_ + line_->GetHeight();
  }

  public: float line_top() const {
    return line_top_;
  }

  public: LineWithTop Next() const {
    ASSERT(line_);
    return LineWithTop(line_->GetNext(), line_bottom());
  }
};

//////////////////////////////////////////////////////////////////////
//
// LineCopier
//
class LineCopier {
  private: typedef Page::Line Line;

  private: const gfx::RectF rect_;
  private: const gfx::Graphics& gfx_;
  private: const common::OwnPtr<gfx::Bitmap> screen_bitmap_;
  private: std::vector<const LineWithTop> lines_;

  public: LineCopier(const gfx::Graphics& gfx, const gfx::RectF& dst_rect,
                     const gfx::RectF& src_rect, const Line* start_line)
      : gfx_(gfx), rect_(dst_rect),
        screen_bitmap_(CreateBitmap(gfx, dst_rect, src_rect)) {
    if (!screen_bitmap_)
      return;
    auto runner = start_line;
    auto runner_top = src_rect.top;
    while (runner) {
      auto const runner_bottom = runner_top + runner->GetHeight();
      if (runner_bottom > src_rect.bottom) {
        // |runner| is partially on screen.
        break;
      }
      lines_.push_back(LineWithTop(runner, runner_top));
      runner_top = runner_bottom;
      runner = runner->GetNext();
    }
  }

  private: static common::OwnPtr<gfx::Bitmap> CreateBitmap(
      const gfx::Graphics& gfx,
      const gfx::RectF& rect, const gfx::RectF& src_rect) {
    // TODO: We should allow copy bitmap from large screen to small screen.
    gfx::RectU screen_rect(gfx->GetPixelSize());
    if (static_cast<float>(screen_rect.width()) != rect.width() ||
        static_cast<float>(screen_rect.height()) != rect.height() ||
        static_cast<float>(screen_rect.width()) != src_rect.width() ||
        static_cast<float>(screen_rect.height()) != src_rect.height()) {
      return std::move(common::OwnPtr<gfx::Bitmap>());
    }

    common::OwnPtr<gfx::Bitmap> bitmap(new gfx::Bitmap(gfx));
    COM_VERIFY((*bitmap)->CopyFromRenderTarget(nullptr, gfx, &screen_rect));
    return std::move(bitmap);
  }

  private: void Copy(float dst_top, float dst_bottom, float src_top) const {
    auto const src_bottom = src_top + dst_bottom - dst_top;
    ASSERT(src_bottom <= rect_.bottom);

    auto const height = std::min(std::min(rect_.bottom, dst_bottom) - dst_top,
                                 std::min(rect_.bottom, src_bottom) - src_top);

    gfx::RectF dst_rect(0.0f, dst_top, rect_.right, dst_top + height);
    gfx::RectF src_rect(0.0f, src_top, rect_.right, src_top + height);
    ASSERT(dst_rect.size() == src_rect.size());

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

  private: LineWithTop FindSameLine(const LineWithTop& line) const {
    for (auto present : lines_) {
      if (present->Equal(line))
        return present;
    }
    return LineWithTop();
  }

  public: LineWithTop TryCopy(const LineWithTop& new_start) const {
    auto present_start = FindSameLine(new_start);
    if (!present_start)
      return LineWithTop();
    auto new_last = new_start;
    auto new_runner = new_start.Next();
    auto present_runner = present_start.Next();
    while (new_runner && present_runner) {
      if (present_runner.line_bottom() > rect_.bottom ||
          !new_runner->Equal(present_runner)) {
        break;
      }
      new_last = new_runner;
      ++new_runner;
      ++present_runner;
    }

    if (new_start.line_top() != present_start.line_top()) {
      Copy(new_start.line_top(), new_last.line_bottom(),
           present_start.line_top());
    }
    return new_last;
  }

  DISALLOW_COPY_AND_ASSIGN(LineCopier);
};

} // namespace

bool Page::Render(const gfx::Graphics& gfx) {
  ASSERT(!m_oFormatBuf.rect().is_empty());
  gfx::Graphics::AxisAlignedClipScope clip_scope(gfx, m_oFormatBuf.rect());
  auto number_of_rendering = 0;
  LineWithTop format_line(m_oFormatBuf.GetFirst(), m_oFormatBuf.top());
  LineWithTop screen_line(m_oScreenBuf.GetFirst(), m_oScreenBuf.top());
  while (format_line && screen_line) {
    if (format_line != screen_line)
      break;
    ++format_line;
    ++screen_line;
  }

  if (format_line) {
    #if DEBUG_RENDER
      DEBUG_PRINTF("start from %d\n",
          static_cast<int>(format_line.line_top() * 100));
    #endif

    LineCopier line_copier(gfx, m_oFormatBuf.rect(), m_oScreenBuf.rect(),
                           m_oScreenBuf.GetFirst());
    while (format_line) {
      auto const last_copy_line = line_copier.TryCopy(format_line);
      if (last_copy_line) {
        format_line = last_copy_line.Next();
        continue;
      }

      ++number_of_rendering;
      format_line->Render(gfx, gfx::PointF(m_oFormatBuf.left(),
                                           format_line.line_top()));
      fillRight(gfx, format_line, format_line.line_top());
      ++format_line;
    }
  }

  fillBottom(gfx, format_line.line_top());

  // Update m_oScreenBuf for next rendering.
  {
    auto const hHeap = m_oScreenBuf.Reset(m_oFormatBuf.rect());
    for (const auto& line : m_oFormatBuf.lines()) {
      m_oScreenBuf.Append(line.Copy(hHeap));
    }
  }

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

void Page::Reset() {
  m_nModfTick = -1;
  m_oScreenBuf.Reset(gfx::RectF());
}

bool Page::ScrollDown(const gfx::Graphics& gfx) {
  if (!m_lStart) {
    // This page shows start of buffer.
    return false;
  }

  auto const pLine = m_oFormatBuf.GetHeight() < m_oFormatBuf.height() ?
      m_oFormatBuf.NewLine() : m_oFormatBuf.ScrollDown();
  if (!pLine) {
    // This page shows only one line.
    return false;
  }

  auto const lGoal  = m_lStart - 1;
  auto const lStart = m_pBuffer->ComputeStartOf(Unit_Line, lGoal);
  Formatter formatter(gfx, m_oFormatBuf.GetHeap(), this, lStart);

  do {
    pLine->Reset();
    formatter.FormatLine(pLine);
  } while (lGoal >= pLine->GetEnd());

  m_oFormatBuf.Prepend(pLine);

  while (m_oFormatBuf.GetHeight() > m_oFormatBuf.height()) {
    auto const pLast = m_oFormatBuf.ScrollDown();
    if (!pLast)
      break;
    pLast->Discard();
  }

  m_lStart = GetFirstLine()->GetStart();
  m_lEnd   = GetLastLine()->GetEnd();
  return true;
}

bool Page::ScrollToPosn(const gfx::Graphics& gfx, Posn lPosn) {
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
    lStart = m_pBuffer->ComputeStartOf(Unit_Line, lStart - 1);
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

bool Page::ScrollUp(const gfx::Graphics& gfx) {
  // Note: We should scroll up if page shows end of buffer. Since,
  // the last line may not be fully visible.

  // Recycle the first line.
  auto const pLine = m_oFormatBuf.ScrollUp();
  if (!pLine) {
    // This page shows only one line.
    return false;
  }

  pLine->Reset();

  Formatter oFormatter(gfx, m_oFormatBuf.GetHeap(), this,
                       GetLastLine()->GetEnd());

  auto const fMore = oFormatter.FormatLine(pLine);
  m_oFormatBuf.Append(pLine);

  auto const cyPage = m_oFormatBuf.height();
  while (m_oFormatBuf.GetHeight() > cyPage) {
    auto const pFirst = m_oFormatBuf.ScrollUp();
    if (!pFirst)
      break;
    pFirst->Discard();
  }

  m_lStart = GetFirstLine()->GetStart();
  m_lEnd   = GetLastLine()->GetEnd();

  return fMore;
}

Page::DisplayBuffer::DisplayBuffer()
    : m_cy(0),
      m_hObjHeap(nullptr) {
}

Page::DisplayBuffer::~DisplayBuffer() {
  #if DEBUG_HEAP
    DEBUG_PRINTF("%p: heap=%p\n", this, m_hObjHeap);
  #endif // DEBUG_HEAP

  if (m_hObjHeap)
    ::HeapDestroy(m_hObjHeap);
}

void Page::DisplayBuffer::Append(Line* pLine) {
  lines_.Append(pLine);
  m_cy += pLine->GetHeight();
}

void* Page::DisplayBuffer::Alloc(size_t cb) {
  return ::HeapAlloc(m_hObjHeap, 0, cb);
}

Page::Line* Page::DisplayBuffer::NewLine() {
  return new(m_hObjHeap) Line(m_hObjHeap);
}

void Page::DisplayBuffer::Prepend(Line* line) {
  lines_.Prepend(line);
  m_cy += line->GetHeight();
}

HANDLE Page::DisplayBuffer::Reset(const gfx::RectF& new_rect) {
  #if DEBUG_DISPBUF
    DEBUG_PRINTF("%p " DEBUG_RECTF_FORMAT " to " DEBUG_RECTF_FORMAT "\n",
        this, DEBUG_RECTF_ARG(rect()), DEBUG_RECTF_ARG(new_rect));
  #endif
  #if DEBUG_HEAP
    DEBUG_PRINTF("%p: heap=%p\n", this, m_hObjHeap);
  #endif // DEBUG_HEAP

  if (m_hObjHeap)
    ::HeapDestroy(m_hObjHeap);

  m_hObjHeap = ::HeapCreate(HEAP_NO_SERIALIZE, 0, 0);
  lines_.DeleteAll();
  m_cy = 0;
  rect_ = new_rect;

  #if DEBUG_HEAP
    DEBUG_PRINTF("%p: new heap=%p\n", this, m_hObjHeap);
  #endif // DEBUG_HEAP

  return m_hObjHeap;
}

Page::Line* Page::DisplayBuffer::ScrollDown() {
  if (lines_.IsEmpty())
    return nullptr;

  auto const line = GetLast();
  lines_.Delete(line);
  m_cy -= line->GetHeight();
  return line;
}

Page::Line* Page::DisplayBuffer::ScrollUp() {
  if (lines_.IsEmpty())
    return nullptr;

  auto const line = GetFirst();
  lines_.Delete(line);
  m_cy -= line->GetHeight();
  return line;
}

//////////////////////////////////////////////////////////////////////
//
// Line
//
Page::Line::Line(HANDLE hHeap)
    : m_cwch(0),
      m_nHash(0),
      m_hObjHeap(hHeap),
      m_iHeight(0),
      m_iWidth(0),
      m_lStart(0),
      m_lEnd(0),
      m_pwch(nullptr) {
  ASSERT(m_hObjHeap);
}

Page::Line::Line(const Line& other, HANDLE hHeap)
    : m_cwch(other.m_cwch),
      m_nHash(other.m_nHash),
      m_hObjHeap(hHeap),
      m_iHeight(other.m_iHeight),
      m_iWidth(other.m_iWidth),
      m_lStart(other.m_lStart),
      m_lEnd(other.m_lEnd),
      m_pwch(reinterpret_cast<char16*>(
                ::HeapAlloc(m_hObjHeap, 0, sizeof(char16) * m_cwch))) {
  ASSERT(m_pwch);
  myCopyMemory(m_pwch, other.m_pwch, sizeof(char16) * m_cwch);
}

Page::Line* Page::Line::Copy(HANDLE hHeap) const {
  auto const pLine = new(hHeap) Line(*this, hHeap);
  ASSERT(pLine->m_pwch);
  for (const auto& cell : cells()) {
    auto const copy = cell.Copy(hHeap, pLine->m_pwch);
    pLine->cells_.Append(copy);
  }
  return pLine;
}

void Page::Line::Discard() {
  if (m_pwch)
    ::HeapFree(m_hObjHeap, 0, m_pwch);
  ::HeapFree(m_hObjHeap, 0, this);
}

bool Page::Line::Equal(const Line* other) const {
  if (Hash() != other->Hash())
    return false;
  auto other_it = other->cells().begin();
  for (const auto& cell : cells()) {
    if (other_it == other->cells_.end())
      return false;
    if (!cell.Equal(&*other_it))
      return false;
    ++other_it;
  }
  return true;
}

// o Assign real pointer to m_pwch.
// o Adjust line cell height.
void Page::Line::Fix(float iDescent) {
  auto const pwch = m_pwch;
  auto cx = 0.0f;
  for (auto& cell : cells()) {
    auto const lEnd = cell.Fix(pwch, m_iHeight, iDescent);
    if (lEnd >= 0)
      m_lEnd = lEnd;
    cx += cell.GetWidth();
  }
  m_iWidth  = cx;
}

uint Page::Line::Hash() const {
  if (m_nHash)
    return m_nHash;
  for (const auto& cell : cells()) {
    m_nHash <<= 5;
    m_nHash ^= cell.Hash();
    m_nHash >>= 3;
  }
  return m_nHash;
}

Posn Page::Line::MapXToPosn(const gfx::Graphics& gfx, float xGoal) const {
  auto xCell = 0.0f;
  auto lPosn = GetEnd() - 1;
  for (const auto& cell : cells()) {
    auto const x = xGoal - xCell;
    xCell += cell.m_cx;
    auto const lMap = cell.MapXToPosn(gfx, x);
    if (lMap >= 0)
      lPosn = lMap;
    if (x >= 0 && x < cell.m_cx)
      break;
  }
  return lPosn;
}

void Page::Line::Render(const gfx::Graphics& gfx,
                        const gfx::PointF& left_top) const {
  auto x = left_top.x;
  for (auto const& cell : cells()) {
    gfx::RectF rect(x, left_top.y, x + cell.m_cx,
                    ::ceilf(left_top.y + cell.m_cy));
    cell.Render(gfx, rect);
    x = rect.right;
  }
  gfx.Flush();
}

void Page::Line::Reset() {
  ASSERT(!GetNext());
  ASSERT(!GetPrev());
  cells_.DeleteAll();
  m_iHeight = 0;
  m_iWidth = 0;
  m_nHash = 0;
  m_lStart = -1;
  m_lEnd = -1;
  m_cwch = 0;
  if (m_pwch) {
    ::HeapFree(m_hObjHeap, 0, m_pwch);
    m_pwch = nullptr;
  }
}
