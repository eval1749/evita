// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>
#include <string>

#include "evita/views/text/text_formatter.h"

#include "base/logging.h"
#include "common/memory/singleton.h"
#include "evita/css/style.h"
#include "evita/css/style_resolver.h"
#include "evita/css/style_selector.h"
#include "evita/text/buffer.h"
#include "evita/text/interval.h"
#include "evita/text/marker.h"
#include "evita/text/marker_set.h"
#include "evita/views/text/render_cell.h"
#include "evita/views/text/render_font.h"
#include "evita/views/text/render_font_set.h"
#include "evita/views/text/render_style.h"
#include "evita/views/text/render_selection.h"
#include "evita/views/text/render_text_line.h"
#include "evita/text/spelling.h"

namespace views {
namespace rendering {

namespace {

const float kLeftMargin = 10.0f;
const int kTabWidth = 4;

float AlignHeightToPixel(float height) {
  return gfx::FactorySet::instance()
      ->AlignToPixel(gfx::SizeF(0.0f, height))
      .height;
}

float AlignWidthToPixel(float width) {
  return width;
}

inline base::char16 toxdigit(int k) {
  if (k <= 9)
    return static_cast<base::char16>(k + '0');
  return static_cast<base::char16>(k - 10 + 'A');
}

gfx::ColorF CssColorToColorF(const css::Color& color) {
  return gfx::ColorF(static_cast<float>(color.red()) / 255,
                     static_cast<float>(color.green()) / 255,
                     static_cast<float>(color.blue()) / 255, color.alpha());
}

const Font* GetFont(const css::Style& style) {
  return FontSet::GetFont(style, 'x');
}

RenderStyle GetRenderStyle(const css::Style& style) {
  return RenderStyle(style, *GetFont(style));
}

RenderStyle MakeRenderStyle(const css::Style& style, const Font* font) {
  return RenderStyle(style, font);
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// TextScanner
// Enumerator for characters and interval
//
class TextFormatter::TextScanner final {
 public:
  explicit TextScanner(const text::Buffer* buffer);
  ~TextScanner() = default;

  const common::AtomicString& spelling() const;
  const css::StyleResolver* style_resolver() const {
    return buffer_->style_resolver();
  }
  const common::AtomicString& syntax() const;
  text::Posn text_offset() const { return text_offset_; }
  void set_text_offset(text::Posn new_text_offset) {
    text_offset_ = new_text_offset;
  }

  bool AtEnd() const;
  base::char16 GetChar();
  const css::Style& GetStyle();
  void Next();

 private:
  base::char16 buffer_cache_[80];
  const text::Buffer* const buffer_;
  text::Posn buffer_cache_end_;
  text::Posn buffer_cache_start_;
  text::Interval* interval_;
  mutable const text::Marker* spelling_marker_;
  mutable const text::Marker* syntax_marker_;
  text::Posn text_offset_;

  DISALLOW_COPY_AND_ASSIGN(TextScanner);
};

TextFormatter::TextScanner::TextScanner(const text::Buffer* buffer)
    : buffer_(buffer),
      buffer_cache_end_(0),
      buffer_cache_start_(0),
      interval_(nullptr),
      spelling_marker_(nullptr),
      syntax_marker_(nullptr),
      text_offset_(0) {}

const common::AtomicString& TextFormatter::TextScanner::spelling() const {
  if (!spelling_marker_ || text_offset_ >= spelling_marker_->end()) {
    spelling_marker_ =
        buffer_->spelling_markers()->GetLowerBoundMarker(text_offset_);
  }
  return spelling_marker_ && spelling_marker_->Contains(text_offset_)
             ? spelling_marker_->type()
             : common::AtomicString::Empty();
}

const common::AtomicString& TextFormatter::TextScanner::syntax() const {
  if (!syntax_marker_ || text_offset_ >= syntax_marker_->end()) {
    syntax_marker_ =
        buffer_->syntax_markers()->GetLowerBoundMarker(text_offset_);
  }
  return syntax_marker_ && syntax_marker_->Contains(text_offset_)
             ? syntax_marker_->type()
             : common::AtomicString::Empty();
}

bool TextFormatter::TextScanner::AtEnd() const {
  DCHECK_LE(text_offset_, buffer_->GetEnd());
  return text_offset_ == buffer_->GetEnd();
}

base::char16 TextFormatter::TextScanner::GetChar() {
  if (AtEnd())
    return 0;
  if (text_offset_ < buffer_cache_start_ || text_offset_ >= buffer_cache_end_) {
    buffer_cache_start_ = text_offset_;
    buffer_cache_end_ = std::min(
        static_cast<text::Posn>(buffer_cache_start_ + arraysize(buffer_cache_)),
        buffer_->GetEnd());
    buffer_->GetText(buffer_cache_, buffer_cache_start_, buffer_cache_end_);
  }
  DCHECK_GE(text_offset_, buffer_cache_start_);
  DCHECK_LT(text_offset_, buffer_cache_end_);
  return buffer_cache_[text_offset_ - buffer_cache_start_];
}

const css::Style& TextFormatter::TextScanner::GetStyle() {
  if (AtEnd())
    return buffer_->GetDefaultStyle();
  if (!interval_ || !interval_->Contains(text_offset_))
    interval_ = buffer_->GetIntervalAt(text_offset_);
  return interval_->style();
}

void TextFormatter::TextScanner::Next() {
  if (AtEnd())
    return;
  ++text_offset_;
}

//////////////////////////////////////////////////////////////////////
//
// TextFormatter
//
TextFormatter::TextFormatter(const text::Buffer* text_buffer,
                             text::Posn text_offset,
                             const gfx::RectF& bounds,
                             float zoom)
    : bounds_(bounds),
      default_render_style_(GetRenderStyle(text_buffer->GetDefaultStyle())),
      text_scanner_(new TextScanner(text_buffer)),
      zoom_(zoom) {
  DCHECK(!bounds_.empty());
  DCHECK_GT(zoom_, 0.0f);
  text_scanner_->set_text_offset(text_offset);
}

TextFormatter::~TextFormatter() {}

text::Posn TextFormatter::text_offset() const {
  return text_scanner_->text_offset();
}

void TextFormatter::set_text_offset(text::Posn new_text_offset) {
  return text_scanner_->set_text_offset(new_text_offset);
}

// Returns true if more contents is available, otherwise returns false.
TextLine* TextFormatter::FormatLine(text::Posn text_offset) {
  text_scanner_->set_text_offset(text_offset);
  return FormatLine();
}

TextLine* TextFormatter::FormatLine() {
  DCHECK(!bounds_.empty());
  auto const line = new TextLine();
  line->set_start(text_scanner_->text_offset());

  auto x = bounds_.left;
  auto descent = 0.0f;
  auto ascent = 0.0f;

  Cell* cell;

  // Left margin
  {
    auto const cyMinHeight = 1.0f;

    cell = new FillerCell(default_render_style_, kLeftMargin, cyMinHeight);
    line->AddCell(cell);
    x += kLeftMargin;
  }

  for (;;) {
    if (text_scanner_->AtEnd()) {
      cell = FormatMarker(TextMarker::EndOfDocument);
      break;
    }

    auto const wch = text_scanner_->GetChar();
    if (wch == 0x0A) {
      cell = FormatMarker(TextMarker::EndOfLine);
      text_scanner_->Next();
      break;
    }

    auto const width = cell->width();
    cell = FormatChar(cell, x, wch);
    if (!cell) {
      cell = FormatMarker(TextMarker::LineWrap);
      break;
    }

    text_scanner_->Next();

    if (line->last_cell() == cell) {
      x -= width;
    } else {
      line->AddCell(cell);
    }

    x += cell->width();
    descent = std::max(cell->descent(), descent);
    ascent = std::max(cell->height() - cell->descent(), ascent);
  }

  // We have at least one cell.
  // o end of buffer: End-Of-Buffer MarkerCell
  // o end of line: End-Of-Line MarkerCell
  // o wrapped line: Warp MarkerCell
  DCHECK(cell);
  line->AddCell(cell);

  x += cell->width();
  descent = std::max(cell->descent(), descent);
  ascent = std::max(cell->height() - cell->descent(), ascent);
  line->Fix(AlignHeightToPixel(ascent), AlignHeightToPixel(descent));
  return line;
}

Cell* TextFormatter::FormatChar(Cell* previous_cell,
                                float x,
                                base::char16 wch) {
  auto const lPosn = text_scanner_->text_offset();
  auto style = text_scanner_->GetStyle();

  auto const spelling = text_scanner_->spelling();
  if (!spelling.empty()) {
    style.Merge(
        text_scanner_->style_resolver()->ResolveWithoutDefaults(spelling));
  }

  auto const syntax = text_scanner_->syntax();
  if (!syntax.empty()) {
    style.Merge(
        text_scanner_->style_resolver()->ResolveWithoutDefaults(syntax));
  }

  style.Merge(
      text_scanner_->style_resolver()->Resolve(css::StyleSelector::defaults()));
  style.set_font_size(style.font_size() * zoom_);

  if (wch == 0x09) {
    style.OverrideBy(text_scanner_->style_resolver()->ResolveWithoutDefaults(
        css::StyleSelector::end_of_file_marker()));
    auto const font = FontSet::GetFont(style, 'x');
    auto const widthTab =
        AlignWidthToPixel(font->GetCharWidth(' ')) * kTabWidth;
    auto const x2 = (x + widthTab - kLeftMargin) / widthTab * widthTab;
    auto const width = (x2 + kLeftMargin) - x;
    auto const width_of_M = AlignWidthToPixel(font->GetCharWidth('M'));
    if (previous_cell && x2 + width_of_M > bounds_.right)
      return nullptr;

    auto const height = AlignHeightToPixel(font->height());
    return new MarkerCell(MakeRenderStyle(style, font), width, height, lPosn,
                          TextMarker::Tab);
  }

  auto const font =
      wch < 0x20 || wch == 0xFEFF ? nullptr : FontSet::GetFont(style, wch);

  if (!font) {
    style.OverrideBy(text_scanner_->style_resolver()->ResolveWithoutDefaults(
        css::StyleSelector::end_of_file_marker()));
    auto const font2 = FontSet::GetFont(style, 'u');
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

    auto const width = font2->GetTextWidth(string) + 4;
    auto const char_width = font2->GetCharWidth('M');
    if (previous_cell && x + width + char_width > bounds_.right)
      return nullptr;
    auto const height = AlignHeightToPixel(font2->height());
    return new UnicodeCell(MakeRenderStyle(style, font2), width, height, lPosn,
                           string);
  }

  auto render_style = MakeRenderStyle(style, font);
  auto const width = AlignWidthToPixel(font->GetCharWidth(wch));
  if (previous_cell) {
    auto const width_of_M = AlignWidthToPixel(font->GetCharWidth('M'));
    if (x + width + width_of_M > bounds_.right) {
      // We doesn't have enough room for a char in the line.
      return nullptr;
    }

    if (previous_cell->Merge(render_style, width)) {
      previous_cell->as<TextCell>()->AddChar(wch);
      return previous_cell;
    }
  }

  auto const height = AlignHeightToPixel(font->height());
  return new TextCell(render_style, width, height, lPosn,
                      base::string16(1u, wch));
}

Cell* TextFormatter::FormatMarker(TextMarker marker_name) {
  auto style = text_scanner_->GetStyle();
  style.Merge(
      text_scanner_->style_resolver()->Resolve(css::StyleSelector::defaults()));
  style.OverrideBy(text_scanner_->style_resolver()->ResolveWithoutDefaults(
      css::StyleSelector::end_of_line_marker()));
  style.set_font_size(style.font_size() * zoom_);

  auto const font = FontSet::GetFont(style, 'x');
  auto const width = AlignWidthToPixel(font->GetCharWidth('x'));
  auto const height = AlignHeightToPixel(font->height());
  return new MarkerCell(MakeRenderStyle(style, font), width, height,
                        text_scanner_->text_offset(), marker_name);
}

TextSelection TextFormatter::FormatSelection(
    const text::Buffer* buffer,
    const TextSelectionModel& selection_model) {
  const auto& style = buffer->style_resolver()->ResolveWithoutDefaults(
      selection_model.disabled() ? css::StyleSelector::inactive_selection()
                                 : css::StyleSelector::active_selection());
  return TextSelection(selection_model, CssColorToColorF(style.bgcolor()));
}

}  // namespace rendering
}  // namespace views
