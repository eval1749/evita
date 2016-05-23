// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>
#include <string>

#include "evita/text/layout/text_formatter.h"

#include "base/logging.h"
#include "base/trace_event/trace_event.h"
#include "evita/css/selector.h"
#include "evita/css/selector_builder.h"
#include "evita/gfx/direct2d_factory_win.h"
#include "evita/gfx/font.h"
#include "evita/gfx/font_face.h"
#include "evita/text/layout/known_names.h"
#include "evita/text/layout/text_format_context.h"
#include "evita/text/models/buffer.h"
#include "evita/text/models/marker.h"
#include "evita/text/models/marker_set.h"
// TODO(eval1749): We should have "evita/text/layout/public/text_marker.h" to
// avoid include "inline_box.h" in "text_formatter.cc".
#include "evita/text/layout/line/inline_box.h"
#include "evita/text/layout/line/line_builder.h"
#include "evita/text/layout/line/root_inline_box.h"
#include "evita/text/layout/render_selection.h"
#include "evita/text/models/spelling.h"
#include "evita/text/style/computed_style.h"
#include "evita/text/style/computed_style_builder.h"
#include "evita/text/style/style_tree.h"

namespace layout {

namespace {

// TODO(eval1749): We should retrieve |kLeftMargin| from CSS.
const float kLeftMargin = 10.0f;
const auto kMinHeight = 1.0f;
const int kTabWidth = 4;

// TODO(eval1749): We should move |AlignHeightToPixel()| to another place
// to share code.
float AlignHeightToPixel(float height) {
  return gfx::Direct2DFactory::GetInstance()
      ->AlignToPixel(gfx::SizeF(0.0f, height))
      .height;
}

// TODO(eval1749): We should move |AlignWidthToPixel()| to another place
// to share code.
float AlignWidthToPixel(float width) {
  return width;
}

const ComputedStyle& ComputeDefaultStyle(const StyleTree& style_tree) {
  const auto& selector =
      css::Selector::Builder().SetTagName(L"default").Build();
  return style_tree.ComputedStyleOf(selector);
}

base::char16 IntToHex(int k) {
  DCHECK_GE(k, 0);
  DCHECK_LE(k, 15);
  if (k <= 9)
    return static_cast<base::char16>(k + '0');
  return static_cast<base::char16>(k - 10 + 'A');
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// TextScanner
// Enumerator for characters and markers
//
class TextFormatter::TextScanner final {
 public:
  TextScanner(const text::Buffer& buffer, const text::MarkerSet& markers);
  ~TextScanner() = default;

  base::AtomicString highlight() const;
  base::AtomicString spelling() const;
  base::AtomicString syntax() const;
  text::Offset text_offset() const { return text_offset_; }
  void set_text_offset(text::Offset new_text_offset) {
    text_offset_ = new_text_offset;
  }

  bool AtEnd() const;
  base::char16 GetChar();
  void Next();

 private:
  base::char16 buffer_cache_[80];
  const text::Buffer& buffer_;
  text::Offset buffer_cache_end_;
  text::Offset buffer_cache_start_;
  const text::MarkerSet& highlight_markers_;
  mutable const text::Marker* highlight_marker_;
  mutable const text::Marker* spelling_marker_;
  mutable const text::Marker* syntax_marker_;
  text::Offset text_offset_;

  DISALLOW_COPY_AND_ASSIGN(TextScanner);
};

TextFormatter::TextScanner::TextScanner(
    const text::Buffer& buffer,
    const text::MarkerSet& highlight_markers)
    : buffer_(buffer),
      buffer_cache_end_(0),
      buffer_cache_start_(0),
      highlight_markers_(highlight_markers),
      highlight_marker_(nullptr),
      spelling_marker_(nullptr),
      syntax_marker_(nullptr),
      text_offset_(0) {
  DCHECK_EQ(&buffer, &highlight_markers.buffer());
}

base::AtomicString TextFormatter::TextScanner::highlight() const {
  if (!highlight_marker_ || text_offset_ >= highlight_marker_->end()) {
    highlight_marker_ = highlight_markers_.GetLowerBoundMarker(text_offset_);
  }
  return highlight_marker_ && highlight_marker_->Contains(text_offset_)
             ? highlight_marker_->type()
             : base::AtomicString();
}

base::AtomicString TextFormatter::TextScanner::spelling() const {
  if (!spelling_marker_ || text_offset_ >= spelling_marker_->end()) {
    spelling_marker_ =
        buffer_.spelling_markers()->GetLowerBoundMarker(text_offset_);
  }
  return spelling_marker_ && spelling_marker_->Contains(text_offset_)
             ? spelling_marker_->type()
             : base::AtomicString();
}

base::AtomicString TextFormatter::TextScanner::syntax() const {
  if (!syntax_marker_ || text_offset_ >= syntax_marker_->end()) {
    syntax_marker_ =
        buffer_.syntax_markers()->GetLowerBoundMarker(text_offset_);
  }
  return syntax_marker_ && syntax_marker_->Contains(text_offset_)
             ? syntax_marker_->type()
             : base::AtomicString();
}

bool TextFormatter::TextScanner::AtEnd() const {
  DCHECK_LE(text_offset_, buffer_.GetEnd());
  return text_offset_ == buffer_.GetEnd();
}

base::char16 TextFormatter::TextScanner::GetChar() {
  if (AtEnd())
    return 0;
  if (text_offset_ < buffer_cache_start_ || text_offset_ >= buffer_cache_end_) {
    buffer_cache_start_ = text_offset_;
    buffer_cache_end_ = std::min(
        buffer_cache_start_ + text::OffsetDelta(arraysize(buffer_cache_)),
        buffer_.GetEnd());
    buffer_.GetText(buffer_cache_, buffer_cache_start_, buffer_cache_end_);
  }
  DCHECK_GE(text_offset_, buffer_cache_start_);
  DCHECK_LT(text_offset_, buffer_cache_end_);
  return buffer_cache_[text_offset_ - buffer_cache_start_];
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
TextFormatter::TextFormatter(const TextFormatContext& context)
    : bounds_(context.bounds()),
      default_computed_style_(ComputeDefaultStyle(context.style_tree())),
      line_start_(context.line_start()),
      style_tree_(context.style_tree()),
      text_scanner_(new TextScanner(context.buffer(), context.markers())),
      zoom_(context.zoom()) {
  DCHECK(!bounds_.empty());
  DCHECK_GT(zoom_, 0.0f);
  DCHECK_EQ(line_start_, context.buffer().ComputeStartOfLine(context.offset()));
  text_scanner_->set_text_offset(context.offset());
}

TextFormatter::~TextFormatter() {}

text::Offset TextFormatter::text_offset() const {
  return text_scanner_->text_offset();
}

const ComputedStyle& TextFormatter::ComputedStyleOf(
    const css::Selector& selector) const {
  return style_tree_.ComputedStyleOf(selector);
}

void TextFormatter::DidFormat(const RootInlineBox* line) {
  line_start_ = line->IsEndOfLine() ? line->text_end() : line->line_start();
  text_scanner_->set_text_offset(line->text_end());
}

const gfx::Font* TextFormatter::FontFor(const ComputedStyle& style,
                                        base::char16 char_code) const {
  DCHECK(!style.fonts().empty());
  for (const auto& font : style.fonts()) {
    if (font->HasCharacter(char_code))
      return font;
  }
  return nullptr;
}

std::unique_ptr<RootInlineBox> TextFormatter::FormatLine() {
  TRACE_EVENT0("views", "TextFormatter::FormatLine");
  DCHECK(!bounds_.empty());

  LineBuilder line_builder(line_start_, text_scanner_->text_offset(),
                           bounds_.width());
  line_builder.AddFillerBox(default_computed_style_, kLeftMargin, kMinHeight,
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
      line_start_ = text_scanner_->text_offset();
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
  css::Selector::Builder selectorBuilder;
  if (char_code < 0x20 || char_code == 0xFEFF) {
    selectorBuilder.SetTagName(KNOWN_NAME_OF(marker));
  } else {
    const auto& syntax = text_scanner_->syntax();
    if (syntax.empty())
      selectorBuilder.SetTagName(KNOWN_NAME_OF(normal));
    else
      selectorBuilder.SetTagName(syntax);
  }

  const auto& spelling = text_scanner_->spelling();
  if (!spelling.empty())
    selectorBuilder.AddClass(spelling);

  const auto highlight = text_scanner_->highlight();
  if (!highlight.empty())
    selectorBuilder.AddClass(highlight);

  if (char_code == 0x09) {
    const auto& style = ComputedStyleOf(selectorBuilder.Build());
    return FormatTab(line_builder, style);
  }

  if (char_code < 0x20 || char_code == 0xFEFF) {
    selectorBuilder.AddClass(KNOWN_NAME_OF(control));
    const auto& style = ComputedStyleOf(selectorBuilder.Build());
    return FormatMissing(line_builder, style, char_code);
  }

  const auto& style = ComputedStyleOf(selectorBuilder.Build());
  if (const auto* font = FontFor(style, char_code)) {
    return line_builder->TryAddChar(style, *font, text_scanner_->text_offset(),
                                    char_code);
  }

  selectorBuilder.AddClass(KNOWN_NAME_OF(missing));
  const auto& style2 = ComputedStyleOf(selectorBuilder.Build());
  return FormatMissing(line_builder, style2, char_code);
}

void TextFormatter::FormatMarker(LineBuilder* line_builder,
                                 TextMarker marker_name,
                                 text::OffsetDelta length) {
  css::Selector::Builder selectorBuilder;
  selectorBuilder.SetTagName(KNOWN_NAME_OF(marker));
  const auto& style = ComputedStyleOf(selectorBuilder.Build());
  const auto* font = FontFor(style, 'x');
  const auto width = AlignWidthToPixel(font->GetCharWidth('x'));
  const auto height = AlignHeightToPixel(font->height());
  const auto offset = text_scanner_->text_offset();
  line_builder->AddMarkerBox(style, *font, width, height, offset,
                             offset + length, marker_name);
}

bool TextFormatter::FormatMissing(LineBuilder* line_builder,
                                  const ComputedStyle& style,
                                  base::char16 char_code) {
  const auto* font = FontFor(style, 'u');
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

  const auto width = font->GetTextWidth(string) + 4;
  if (!line_builder->HasRoomFor(width))
    return false;
  const auto height = AlignHeightToPixel(font->height()) + 4;
  const auto offset = text_scanner_->text_offset();
  line_builder->AddCodeUnitBox(style, *font, width, height, offset, string);
  return true;
}

bool TextFormatter::FormatTab(LineBuilder* line_builder,
                              const ComputedStyle& style) {
  const auto* font = FontFor(style, 'x');
  const auto widthTab = AlignWidthToPixel(font->GetCharWidth(' ')) * kTabWidth;
  const auto current_x = line_builder->current_x();
  const auto x2 = (current_x + widthTab - kLeftMargin) / widthTab * widthTab;
  const auto width = (x2 + kLeftMargin) - current_x;
  if (!line_builder->HasRoomFor(width))
    return false;
  const auto height = AlignHeightToPixel(font->height());
  const auto offset = text_scanner_->text_offset();
  line_builder->AddMarkerBox(style, *font, width, height, offset,
                             offset + text::OffsetDelta(1), TextMarker::Tab);
  return true;
}

}  // namespace layout
