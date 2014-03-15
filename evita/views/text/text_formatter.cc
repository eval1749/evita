// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/views/text/text_formatter.h"

#include "base/logging.h"
#include "evita/css/style.h"
#include "evita/css/style_resolver.h"
#include "evita/css/style_selector.h"
#include "evita/text/buffer.h"
#include "evita/text/interval.h"
#include "evita/text/marker.h"
#include "evita/text/marker_set.h"
#include "evita/views/text/render_cell.h"
#include "evita/views/text/render_style.h"
#include "evita/views/text/render_selection.h"
#include "evita/views/text/render_text_block.h"
#include "evita/views/text/render_text_line.h"
#include "evita/text/spelling.h"

namespace views {
namespace rendering {

namespace {

const float cxLeftMargin = 10.0f;
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

Font* GetFont(const gfx::Graphics& gfx, const css::Style& style) {
  return FontSet::Get(gfx, style)->FindFont(gfx, 'x');
}

RenderStyle GetRenderStyle(const gfx::Graphics& gfx,
                           const css::Style& style) {
  return RenderStyle(style, GetFont(gfx, style));
}

css::Style SelectionStyle(text::Buffer* buffer, const Selection& selection) {
  return buffer->style_resolver()->ResolveWithoutDefaults(
      selection.active ? css::StyleSelector::active_selection() :
                         css::StyleSelector::inactive_selection());
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// TextScanner
//  Enumerator for characters and interval
//
class TextFormatter::TextScanner {
  private: Posn m_lBufEnd;
  private: Posn m_lBufStart;
  private: Posn m_lPosn;
  private: text::Buffer* m_pBuffer;
  private: text::Interval* m_pInterval;
  private: mutable const text::Marker* marker_;
  private: char16 m_rgwch[80];
  private: const Selection& selection_;
  private: const css::Style selection_style_;

  public: TextScanner(text::Buffer* buffer, Posn lPosn,
                      const Selection& selection)
      : m_pBuffer(buffer),
        m_lPosn(lPosn),
        marker_(nullptr),
        selection_(selection),
        selection_style_(SelectionStyle(buffer, selection)) {
    m_pInterval = m_pBuffer->GetIntervalAt(m_lPosn);
    DCHECK(m_pInterval);
    fill();
  }

  public: text::Spelling spelling() const;
  public: const css::StyleResolver* style_resolver() const {
    return m_pBuffer->style_resolver();
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

  public: const css::Style& GetStyle() const {
    if (AtEnd())
      return m_pBuffer->GetDefaultStyle();
    DCHECK(m_pInterval);
    return m_pInterval->GetStyle();
  }

  public: RenderStyle MakeRenderStyle(const css::Style& style,
                                      Font* font) const;

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

  DISALLOW_COPY_AND_ASSIGN(TextScanner);

};

text::Spelling TextFormatter::TextScanner::spelling() const {
  if (!marker_ || m_lPosn >= marker_->end())
    marker_ = m_pBuffer->spelling_markers()->GetLowerBoundMarker(m_lPosn);
  return marker_ && marker_->Contains(m_lPosn) ?
      static_cast<text::Spelling>(marker_->type()) : text::Spelling::None;
}

RenderStyle TextFormatter::TextScanner::MakeRenderStyle(
    const css::Style& style, Font* font) const {
  if (m_lPosn < selection_.start || m_lPosn >= selection_.end)
    return RenderStyle(style, font);
  if (selection_style_.bgcolor().alpha() == 1.0f) {
    css::Style style_with_selection(style);
    style_with_selection.OverrideBy(selection_style_);
    return RenderStyle(style_with_selection, font);
  }
  auto render_style = RenderStyle(style, font);
  render_style.set_overlay_color(selection_style_.bgcolor());
  return render_style;
}

//////////////////////////////////////////////////////////////////////
//
// TextFormatter
//
TextFormatter::TextFormatter(const gfx::Graphics& gfx, TextBlock* text_block,
                             text::Buffer* buffer, Posn lStart,
                             const Selection& selection)
    : default_render_style_(GetRenderStyle(gfx, buffer->GetDefaultStyle())),
      default_style_(buffer->GetDefaultStyle()),
      m_gfx(gfx),
      text_block_(text_block),
      text_scanner_(new TextScanner(buffer, lStart, selection)) {
  DCHECK(!text_block_->rect().empty());
}

TextFormatter::~TextFormatter() {
}

void TextFormatter::Format() {
  DCHECK(!text_block_->rect().empty());
  for (;;) {
    auto const pLine = FormatLine();
    DCHECK_GT(pLine->rect().height(), 0.0f);

    text_block_->Append(pLine);

    // Line must have at least one cell other than filler.
    DCHECK_GE(pLine->GetEnd(), pLine->GetStart());

    if (text_block_->GetHeight() >= text_block_->height()) {
      // TextBlock is filled up with lines.
      break;
    }

    if (auto const marker_cell = pLine->cells().back()->as<MarkerCell>()) {
      if (marker_cell->marker_name() == TextMarker::EndOfDocument) {
        // We have no more contents.
        break;
      }
    }
  }

  auto& style = text_block_->text_buffer()->GetDefaultStyle();
  text_block_->set_default_style(RenderStyle(style, nullptr));
  text_block_->Finish();
}

// Returns true if more contents is avaialble, otherwise returns false.
TextLine* TextFormatter::FormatLine() {
  DCHECK(!text_block_->rect().empty());
  auto const pLine = new TextLine();
  pLine->set_start(text_scanner_->GetPosn());

  auto x = text_block_->left();
  auto descent = 0.0f;
  auto ascent  = 0.0f;

  Cell* pCell;

  // Left margin
  {
    auto const cyMinHeight = 1.0f;

    pCell = new FillerCell(default_render_style_, cxLeftMargin, cyMinHeight);
    pLine->AddCell(pCell);
    x += cxLeftMargin;
  }

  for (;;) {
    if (text_scanner_->AtEnd()) {
      pCell = formatMarker(TextMarker::EndOfDocument);
      break;
    }

    auto const wch = text_scanner_->GetChar();

    if (wch == 0x0A) {
      pCell = formatMarker(TextMarker::EndOfLine);
      text_scanner_->Next();
      break;
    }

    auto const cx = pCell->m_cx;

    pCell = formatChar(pCell, x, wch);
    if (!pCell) {
      pCell = formatMarker(TextMarker::LineWrap);
      break;
    }

    text_scanner_->Next();

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

  pLine->Fix(text_block_->left(), text_block_->top() + text_block_->GetHeight(),
             AlignHeightToPixel(m_gfx, ascent),
             AlignHeightToPixel(m_gfx, descent));

  return pLine;
}

//////////////////////////////////////////////////////////////////////
//
// TextFormatter::formatChar
//
Cell* TextFormatter::formatChar(Cell* pPrev, float x, char16 wch) {
  auto const lPosn = text_scanner_->GetPosn();
  auto style = text_scanner_->GetStyle();
  style.Merge(text_scanner_->style_resolver()->Resolve(
    css::StyleSelector::defaults()));

  switch (text_scanner_->spelling()) {
    case text::Spelling::Misspelled:
      style.set_text_decoration(css::TextDecoration::RedWave);
      break;
    case text::Spelling::BadGrammar:
      style.set_text_decoration(css::TextDecoration::GreenWave);
      break;
  }

  if (0x09 == wch) {
    style.OverrideBy(text_scanner_->style_resolver()->ResolveWithoutDefaults(
        css::StyleSelector::end_of_file_marker()));
    auto const pFont = FontSet::Get(m_gfx, style)->FindFont(m_gfx, 'x');
    auto const cxTab = AlignWidthToPixel(m_gfx, pFont->GetCharWidth(' ')) *
                          k_nTabWidth;
    auto const x2 = (x + cxTab - cxLeftMargin) / cxTab * cxTab;
    auto const cx = (x2 + cxLeftMargin) - x;
    auto const cxM = AlignWidthToPixel(m_gfx, pFont->GetCharWidth('M'));
    if (pPrev && x2 + cxM > text_block_->right())
      return nullptr;

    auto const height = AlignHeightToPixel(m_gfx, pFont->height());
    return new MarkerCell(text_scanner_->MakeRenderStyle(style, pFont), cx, height,
                          lPosn, TextMarker::Tab);
  }

  auto const pFont = wch < 0x20 ?
      nullptr :
      FontSet::Get(m_gfx, style)->FindFont(m_gfx, wch);

  if (!pFont) {
    auto const pFont = FontSet::Get(m_gfx, style)->FindFont(m_gfx, 'u');
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
    if (pPrev && x + cxUni + cxM > text_block_->right())
      return nullptr;

    auto const height = AlignHeightToPixel(m_gfx, pFont->height());
    return new UnicodeCell(text_scanner_->MakeRenderStyle(style, pFont), cxUni,
                           height, lPosn, string);
  }

  auto render_style = text_scanner_->MakeRenderStyle(style, pFont);
  auto const cx = AlignWidthToPixel(m_gfx, pFont->GetCharWidth(wch));
  if (pPrev) {
    auto const cxM = AlignWidthToPixel(m_gfx, pFont->GetCharWidth('M'));
    if (x + cx + cxM > text_block_->right()) {
      // We doesn't have enough room for a char in the line.
      return nullptr;
    }

    if (pPrev->Merge(render_style, cx)) {
      pPrev->as<TextCell>()->AddChar(wch);
      return pPrev;
    }
  }

  auto const height = AlignHeightToPixel(m_gfx, pFont->height());
  return new TextCell(render_style, cx, height, lPosn, base::string16(1u, wch));
}

Cell* TextFormatter::formatMarker(TextMarker marker_name) {
  auto style = text_scanner_->GetStyle();
  style.Merge(text_scanner_->style_resolver()->Resolve(
    css::StyleSelector::defaults()));
  style.OverrideBy(text_scanner_->style_resolver()->ResolveWithoutDefaults(
      css::StyleSelector::end_of_line_marker()));

  auto const pFont = FontSet::Get(m_gfx, style)->FindFont(m_gfx, 'x');
  auto const width = AlignWidthToPixel(m_gfx, pFont->GetCharWidth('x'));
  auto const height = AlignHeightToPixel(m_gfx, pFont->height());
  return new MarkerCell(text_scanner_->MakeRenderStyle(style, pFont),
                        width, height, text_scanner_->GetPosn(),
                        marker_name);
}

} // namespace rendering
} // namespace views
