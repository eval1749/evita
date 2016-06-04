// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/text/layout/text_layout_test_base.h"

#include "base/strings/utf_string_conversions.h"
#include "evita/css/selector_parser.h"
#include "evita/css/style.h"
#include "evita/css/style_builder.h"
#include "evita/css/style_sheet.h"
#include "evita/text/layout/text_format_context.h"
#include "evita/text/models/buffer.h"
#include "evita/text/models/marker_set.h"
#include "evita/text/style/style_tree.h"

namespace layout {

using FontFamily = css::FontFamily;

namespace {

css::Selector AsSelector(base::StringPiece text) {
  return css::Selector::Parser().Parse(base::UTF8ToUTF16(text));
}

css::StyleSheet* CreateStyleSheet() {
  auto style_sheet = new css::StyleSheet();
  style_sheet->AppendRule(
      AsSelector("*"),
      std::move(css::StyleBuilder()
                    .SetColor(css::ColorValue::Rgba(0, 0, 0))
                    .SetBackgroundColor(css::ColorValue::Rgba(255, 255, 255))
                    .SetFontSize(10)
                    .SetFontFamily(FontFamily(css::String(L"Consolas, Meiryo")))
                    .Build()));
  return style_sheet;
}

}  // namespace

TextLayoutTestBase::TextLayoutTestBase()
    : bounds_(gfx::PointF(300, 200), gfx::SizeF(100, 50)),
      buffer_(new text::Buffer()),
      markers_(new text::MarkerSet(text::MarkerSet::Kind::Sticky, *buffer_)),
      style_sheet_(CreateStyleSheet()),
      style_tree_(new StyleTree({style_sheet_})) {}

TextFormatContext TextLayoutTestBase::FormatContextFor(
    text::Offset line_start,
    text::Offset offset) const {
  return TextFormatContext(*buffer_, *markers_, *style_tree_, line_start,
                           offset, bounds_, zoom_);
}

TextFormatContext TextLayoutTestBase::FormatContextFor(
    text::Offset offset) const {
  const auto line_start = buffer_->ComputeStartOfLine(offset);
  return FormatContextFor(line_start, offset);
}

TextLayoutTestBase::~TextLayoutTestBase() = default;

}  // namespace layout
