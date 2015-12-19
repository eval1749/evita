// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>
#include <string>

#include "evita/layout/text_formatter.h"

#include "base/logging.h"
#include "base/trace_event/trace_event.h"
#include "common/memory/singleton.h"
#include "evita/css/style.h"
#include "evita/css/style_resolver.h"
#include "evita/css/style_selector.h"
#include "evita/gfx/font.h"
#include "evita/text/buffer.h"
#include "evita/text/interval.h"
#include "evita/text/marker.h"
#include "evita/text/marker_set.h"
// TODO(eval1749): We should have "evita/layout/public/text_marker.h" to avoid
// include "inline_box.h" in "text_formatter.cc".
#include "evita/layout/line/inline_box.h"
#include "evita/layout/line/line_builder.h"
#include "evita/layout/line/root_inline_box.h"
#include "evita/layout/render_font_set.h"
#include "evita/layout/render_style.h"
#include "evita/layout/render_selection.h"
#include "evita/text/spelling.h"

namespace layout {

namespace {

const float kLeftMargin = 10.0f;
const auto kMinHeight = 1.0f;
const int kTabWidth = 4;

// TODO(eval1749): We should move |AlignHeightToPixel()| to another place
// to share code.
float AlignHeightToPixel(float height) {
  return gfx::FactorySet::instance()
      ->AlignToPixel(gfx::SizeF(0.0f, height))
      .height;
}

// TODO(eval1749): We should move |AlignWidthToPixel()| to another place
// to share code.
float AlignWidthToPixel(float width) {
  return width;
}

gfx::ColorF CssColorToColorF(const css::Color& color) {
  return gfx::ColorF(static_cast<float>(color.red()) / 255,
                     static_cast<float>(color.green()) / 255,
                     static_cast<float>(color.blue()) / 255, color.alpha());
}

const gfx::Font* GetFont(const css::Style& style) {
  return FontSet::GetFont(style, 'x');
}

RenderStyle GetRenderStyle(const css::Style& style) {
  return RenderStyle(style, *GetFont(style));
}

base::char16 IntToHex(int k) {
  DCHECK_GE(k, 0);
  DCHECK_LE(k, 15);
  if (k <= 9)
    return static_cast<base::char16>(k + '0');
  return static_cast<base::char16>(k - 10 + 'A');
}

RenderStyle MakeRenderStyle(const css::Style& style, const gfx::Font& font) {
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
  text::Offset text_offset() const { return text_offset_; }
  void set_text_offset(text::Offset new_text_offset) {
    text_offset_ = new_text_offset;
  }

  bool AtEnd() const;
  base::char16 GetChar();
  const css::Style& GetStyle();
  void Next();

 private:
  base::char16 buffer_cache_[80];
  const text::Buffer* const buffer_;
  text::Offset buffer_cache_end_;
  text::Offset buffer_cache_start_;
  text::Interval* interval_;
  mutable const text::Marker* spelling_marker_;
  mutable const text::Marker* syntax_marker_;
  text::Offset text_offset_;

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
        buffer_cache_start_ + text::OffsetDelta(arraysize(buffer_cache_)),
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
                             text::Offset text_offset,
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

text::Offset TextFormatter::text_offset() const {
  return text_scanner_->text_offset();
}

void TextFormatter::set_text_offset(text::Offset new_text_offset) {
  return text_scanner_->set_text_offset(new_text_offset);
}

// Returns true if more contents is available, otherwise returns false.
scoped_refptr<RootInlineBox> TextFormatter::FormatLine(
    text::Offset text_offset) {
  text_scanner_->set_text_offset(text_offset);
  return std::move(FormatLine());
}

scoped_refptr<RootInlineBox> TextFormatter::FormatLine() {
  TRACE_EVENT0("views", "TextFormatter::FormatLine");
  DCHECK(!bounds_.empty());

  LineBuilder line_builder(default_render_style_, text_scanner_->text_offset(),
                           bounds_.width());
  line_builder.AddFillerBox(default_render_style_, kLeftMargin, kMinHeight,
                            text_scanner_->text_offset());
  for (;;) {
    if (text_scanner_->AtEnd()) {
      FormatMarker(&line_builder, TextMarker::EndOfDocument,
                   text::OffsetDelta(1));
      break;
    }

    const auto char_code = text_scanner_->GetChar();
    if (char_code == 0x0A) {
      FormatMarker(&line_builder, TextMarker::EndOfLine, text::OffsetDelta(1));
      text_scanner_->Next();
      break;
    }

    if (!FormatChar(&line_builder, char_code)) {
      FormatMarker(&line_builder, TextMarker::LineWrap, text::OffsetDelta(0));
      break;
    }

    text_scanner_->Next();
  }
  return std::move(line_builder.Build());
}

bool TextFormatter::FormatChar(LineBuilder* line_builder,
                               base::char16 char_code) {
  const auto offset = text_scanner_->text_offset();
  auto style = text_scanner_->GetStyle();

  const auto spelling = text_scanner_->spelling();
  if (!spelling.empty()) {
    style.Merge(
        text_scanner_->style_resolver()->ResolveWithoutDefaults(spelling));
  }

  const auto syntax = text_scanner_->syntax();
  if (!syntax.empty()) {
    style.Merge(
        text_scanner_->style_resolver()->ResolveWithoutDefaults(syntax));
  }

  style.Merge(
      text_scanner_->style_resolver()->Resolve(css::StyleSelector::defaults()));
  style.set_font_size(style.font_size() * zoom_);

  if (char_code == 0x09) {
    style.OverrideBy(text_scanner_->style_resolver()->ResolveWithoutDefaults(
        css::StyleSelector::end_of_file_marker()));
    const auto font = FontSet::GetFont(style, 'x');
    const auto widthTab =
        AlignWidthToPixel(font->GetCharWidth(' ')) * kTabWidth;
    const auto current_x = line_builder->current_x();
    const auto x2 = (current_x + widthTab - kLeftMargin) / widthTab * widthTab;
    const auto width = (x2 + kLeftMargin) - current_x;
    if (!line_builder->HasRoomFor(width))
      return false;
    const auto height = AlignHeightToPixel(font->height());
    line_builder->AddMarkerBox(MakeRenderStyle(style, *font), width, height,
                               offset, offset + text::OffsetDelta(1),
                               TextMarker::Tab);
    return true;
  }

  const auto font = char_code < 0x20 || char_code == 0xFEFF
                        ? nullptr
                        : FontSet::GetFont(style, char_code);

  if (!font) {
    style.OverrideBy(text_scanner_->style_resolver()->ResolveWithoutDefaults(
        css::StyleSelector::end_of_file_marker()));
    const auto font2 = FontSet::GetFont(style, 'u');
    base::string16 string;
    if (char_code < 0x20) {
      string.push_back('^');
      string.push_back(static_cast<base::char16>(char_code + 0x40));
    } else {
      string.push_back('u');
      string.push_back(IntToHex((char_code >> 12) & 15));
      string.push_back(IntToHex((char_code >> 8) & 15));
      string.push_back(IntToHex((char_code >> 4) & 15));
      string.push_back(IntToHex((char_code >> 0) & 15));
    }

    const auto width = font2->GetTextWidth(string) + 4;
    if (!line_builder->HasRoomFor(width))
      return false;
    const auto height = AlignHeightToPixel(font2->height()) + 4;
    line_builder->AddCodeUnitBox(MakeRenderStyle(style, *font2), width, height,
                                 offset, string);
    return true;
  }

  return line_builder->TryAddChar(MakeRenderStyle(style, *font), offset,
                                  char_code);
}

void TextFormatter::FormatMarker(LineBuilder* line_builder,
                                 TextMarker marker_name,
                                 text::OffsetDelta length) {
  auto style = text_scanner_->GetStyle();
  style.Merge(
      text_scanner_->style_resolver()->Resolve(css::StyleSelector::defaults()));
  style.OverrideBy(text_scanner_->style_resolver()->ResolveWithoutDefaults(
      css::StyleSelector::end_of_line_marker()));
  style.set_font_size(style.font_size() * zoom_);

  const auto font = FontSet::GetFont(style, 'x');
  const auto width = AlignWidthToPixel(font->GetCharWidth('x'));
  const auto height = AlignHeightToPixel(font->height());
  line_builder->AddMarkerBox(MakeRenderStyle(style, *font), width, height,
                             text_scanner_->text_offset(),
                             text_scanner_->text_offset() + length,
                             marker_name);
}

// TODO(eval1749): We should move |TextFormatter::FormatSelection()| somewhere
// other than her.
TextSelection TextFormatter::FormatSelection(
    const text::Buffer& buffer,
    const TextSelectionModel& selection_model) {
  const auto& style = buffer.style_resolver()->ResolveWithoutDefaults(
      selection_model.disabled() ? css::StyleSelector::inactive_selection()
                                 : css::StyleSelector::active_selection());
  return TextSelection(selection_model, CssColorToColorF(style.bgcolor()));
}

}  // namespace layout
