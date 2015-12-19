// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>
#include <cmath>

#include "evita/layout/line/line_builder.h"

#include "base/logging.h"
#include "evita/gfx/font.h"
#include "evita/layout/line/inline_box.h"
#include "evita/layout/line/root_inline_box.h"

namespace layout {

namespace {

// TODO(eval1749): We should move |AlignHeightToPixel()| to another place
// to share code.
#if 0
float AlignHeightToPixel(float height) {
  return gfx::FactorySet::instance()
      ->AlignToPixel(gfx::SizeF(0.0f, height))
      .height;
}
#else
float AlignHeightToPixel(float height) {
  return height;
}
#endif

}  // namespace

LineBuilder::LineBuilder(const RenderStyle& style,
                         text::Offset text_start,
                         float line_width)
    : line_width_(line_width), style_(style), text_start_(text_start) {}

LineBuilder::~LineBuilder() {}

void LineBuilder::AddBoxInternal(InlineBox* box) {
  boxes_.push_back(box);
  current_x_ += box->width();
  ascent_ = std::max(box->height() - box->descent(), ascent_);
  descent_ = std::max(box->descent(), descent_);
}

void LineBuilder::AddCodeUnitBox(const RenderStyle& style,
                                 float width,
                                 float height,
                                 text::Offset offset,
                                 const base::string16& text) {
  AddTextBoxIfNeeded();
  AddBoxInternal(new InlineUnicodeBox(style, current_x_, width, height,
                                      offset - text_start_, text));
}

void LineBuilder::AddFillerBox(const RenderStyle& style,
                               float width,
                               float height,
                               text::Offset offset) {
  AddTextBoxIfNeeded();
  AddBoxInternal(new InlineFillerBox(style, current_x_, width, height,
                                     offset - text_start_));
}

void LineBuilder::AddMarkerBox(const RenderStyle& style,
                               float width,
                               float height,
                               text::Offset start,
                               text::Offset end,
                               TextMarker marker_name) {
  AddTextBoxIfNeeded();
  AddBoxInternal(new InlineMarkerBox(style, current_x_, width, height,
                                     start - text_start_, end - text_start_,
                                     marker_name));
}

void LineBuilder::AddTextBoxIfNeeded() {
  if (pending_text_.empty())
    return;
  DCHECK_GT(pending_text_width_, 0.0f);
  AddBoxInternal(new InlineTextBox(
      style_, current_x_, pending_text_width_, ::ceil(style_.font().height()),
      current_offset_ - text_start_,
      base::string16(pending_text_.data(), pending_text_.size())));
  pending_text_.clear();
  pending_text_width_ = 0.0f;
}

// We have at least one cell.
// o end of buffer: End-Of-Buffer InlineMarkerBox
// o end of line: End-Of-Line InlineMarkerBox
// o wrapped line: Warp InlineMarkerBox
scoped_refptr<RootInlineBox> LineBuilder::Build() {
  DCHECK(!boxes_.empty());
  const auto end = boxes_.back()->as<InlineMarkerBox>()->end();
  return make_scoped_refptr(new RootInlineBox(
      boxes_, text_start_, text_start_ + end, AlignHeightToPixel(ascent_),
      AlignHeightToPixel(descent_)));
}

bool LineBuilder::HasRoomFor(float width) const {
  DCHECK(!boxes_.empty());
  if (boxes_.size() == 1)
    return true;
  const auto marker_width = ::ceil(style_.font().GetCharWidth('M'));
  return current_x_ + pending_text_width_ + width + marker_width < line_width_;
}

bool LineBuilder::TryAddChar(const RenderStyle& style,
                             text::Offset offset,
                             base::char16 char_code) {
  if (style_ != style) {
    AddTextBoxIfNeeded();
    style_ = style;
  }
  if (pending_text_.empty())
    current_offset_ = offset;
  auto const width = style.font().GetCharWidth(char_code);
  if (!HasRoomFor(width))
    return false;
  pending_text_.push_back(char_code);
  pending_text_width_ += width;
  return true;
}

}  // namespace layout
