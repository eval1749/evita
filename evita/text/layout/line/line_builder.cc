// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>
#include <cmath>

#include "evita/text/layout/line/line_builder.h"

#include "base/logging.h"
#include "evita/gfx/font.h"
#include "evita/text/layout/line/inline_box.h"
#include "evita/text/layout/line/root_inline_box.h"

namespace layout {

namespace {

// TODO(eval1749): We should move |AlignHeightToPixel()| to another place
// to share code.
#if 0
float AlignHeightToPixel(float height) {
  return gfx::Direct2DFactory::GetInstance()
      ->AlignToPixel(gfx::SizeF(0.0f, height))
      .height;
}
#else
float AlignHeightToPixel(float height) {
  return height;
}
#endif

}  // namespace

LineBuilder::LineBuilder(text::Offset line_start,
                         text::Offset text_start,
                         float line_width)
    : line_start_(line_start),
      line_width_(line_width),
      text_start_(text_start) {}

LineBuilder::~LineBuilder() {}

void LineBuilder::AddBoxInternal(InlineBox* box) {
  boxes_.push_back(box);
  current_x_ += box->width();
  ascent_ = std::max(box->height() - box->descent(), ascent_);
  descent_ = std::max(box->descent(), descent_);
}

void LineBuilder::AddCodeUnitBox(const ComputedStyle& style,
                                 const gfx::Font& font,
                                 float width,
                                 float height,
                                 text::Offset offset,
                                 const base::string16& text) {
  AddTextBoxIfNeeded();
  AddBoxInternal(new InlineUnicodeBox(style, font, current_x_, width, height,
                                      offset - text_start_, text));
}

void LineBuilder::AddFillerBox(const ComputedStyle& style,
                               float width,
                               float height,
                               text::Offset offset) {
  AddTextBoxIfNeeded();
  AddBoxInternal(new InlineFillerBox(style, current_x_, width, height,
                                     offset - text_start_));
  font_ = style.fonts()[0];
}

void LineBuilder::AddMarkerBox(const ComputedStyle& style,
                               const gfx::Font& font,
                               float width,
                               float height,
                               text::Offset start,
                               text::Offset end,
                               TextMarker marker_name) {
  AddTextBoxIfNeeded();
  AddBoxInternal(new InlineMarkerBox(style, font, current_x_, width, height,
                                     start - text_start_, end - text_start_,
                                     marker_name));
}

void LineBuilder::AddTextBoxIfNeeded() {
  if (pending_text_.empty())
    return;
  DCHECK_GT(pending_text_width_, 0.0f);
  AddBoxInternal(new InlineTextBox(
      *style_, *font_, current_x_, pending_text_width_, ::ceil(font_->height()),
      current_offset_ - text_start_,
      base::string16(pending_text_.data(), pending_text_.size())));
  pending_text_.clear();
  pending_text_width_ = 0.0f;
}

// We have at least one cell.
// o end of buffer: End-Of-Buffer InlineMarkerBox
// o end of line: End-Of-Line InlineMarkerBox
// o wrapped line: Warp InlineMarkerBox
std::unique_ptr<RootInlineBox> LineBuilder::Build() {
  DCHECK(!boxes_.empty());
  const auto end = boxes_.back()->end();
  return std::make_unique<RootInlineBox>(
      boxes_, line_start_, text_start_, text_start_ + end,
      AlignHeightToPixel(ascent_), AlignHeightToPixel(descent_));
}

bool LineBuilder::HasRoomFor(float width) const {
  DCHECK(!boxes_.empty());
  if (boxes_.size() == 1 && pending_text_.empty())
    return true;
  const auto marker_width = ::ceil(font_->GetCharWidth('M'));
  return current_x_ + pending_text_width_ + width + marker_width < line_width_;
}

bool LineBuilder::TryAddChar(const ComputedStyle& style,
                             const gfx::Font& font,
                             text::Offset offset,
                             base::char16 char_code) {
  if (font_ != &font || style_ != &style) {
    AddTextBoxIfNeeded();
    font_ = &font;
    style_ = &style;
  }
  if (pending_text_.empty())
    current_offset_ = offset;
  auto const width = font_->GetCharWidth(char_code);
  if (!HasRoomFor(width))
    return false;
  pending_text_.push_back(char_code);
  pending_text_width_ += width;
  return true;
}

}  // namespace layout
