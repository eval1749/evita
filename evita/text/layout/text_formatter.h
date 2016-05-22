// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_TEXT_LAYOUT_TEXT_FORMATTER_H_
#define EVITA_TEXT_LAYOUT_TEXT_FORMATTER_H_

#include <memory>

#include "base/macros.h"
#include "evita/gfx/rect_f.h"
#include "evita/text/models/offset.h"
#include "evita/text/style/computed_style.h"

namespace css {
class Selector;
}

namespace gfx {
class Font;
}

namespace text {
class Buffer;
class MarkerSet;
}

namespace layout {

class InlineBox;
class LineBuilder;
class RootInlineBox;
class StyleTree;
class TextFormatContext;
enum class TextMarker;
class TextSelection;
class TextSelectionModel;

class TextFormatter final {
 public:
  explicit TextFormatter(const TextFormatContext& context);
  ~TextFormatter();

  text::Offset text_offset() const;

  void DidFormat(const RootInlineBox* line);
  std::unique_ptr<RootInlineBox> FormatLine();

 private:
  class TextScanner;

  const ComputedStyle& ComputedStyleOf(const css::Selector& selector) const;
  const gfx::Font* FontFor(const ComputedStyle& style,
                           base::char16 char_code) const;
  bool FormatChar(LineBuilder* line_builder, base::char16 char_code);
  void FormatMarker(LineBuilder* line_buffer,
                    TextMarker marker_name,
                    text::OffsetDelta length);
  bool FormatMissing(LineBuilder* line_builder,
                     const ComputedStyle& style,
                     base::char16 char_code);
  bool FormatTab(LineBuilder* line_builder, const ComputedStyle& style);

  const gfx::RectF bounds_;
  const ComputedStyle& default_computed_style_;
  text::Offset line_start_;
  const StyleTree& style_tree_;
  std::unique_ptr<TextScanner> text_scanner_;
  const float zoom_;

  DISALLOW_COPY_AND_ASSIGN(TextFormatter);
};

}  // namespace layout

#endif  // EVITA_TEXT_LAYOUT_TEXT_FORMATTER_H_
