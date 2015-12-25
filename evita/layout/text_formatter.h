// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_LAYOUT_TEXT_FORMATTER_H_
#define EVITA_LAYOUT_TEXT_FORMATTER_H_

#include <memory>

#include "base/macros.h"
#include "evita/gfx/rect_f.h"
#include "evita/layout/render_style.h"
#include "evita/text/offset.h"

namespace text {
class Buffer;
}

namespace layout {

class InlineBox;
class LineBuilder;
class RootInlineBox;
enum class TextMarker;
class TextSelection;
class TextSelectionModel;

class TextFormatter final {
 public:
  TextFormatter(const text::Buffer& buffer,
                text::Offset line_start,
                text::Offset text_offset,
                const gfx::RectF& bounds,
                float zoom);
  ~TextFormatter();

  text::Offset text_offset() const;

  void DidFormat(const RootInlineBox* line);
  std::unique_ptr<RootInlineBox> FormatLine();

  static TextSelection FormatSelection(
      const text::Buffer& buffer,
      const TextSelectionModel& selection_model);

 private:
  class TextScanner;

  bool FormatChar(LineBuilder* line_builder, base::char16 char_code);
  void FormatMarker(LineBuilder* line_buffer,
                    TextMarker marker_name,
                    text::OffsetDelta length);

  const gfx::RectF bounds_;
  RenderStyle default_render_style_;
  text::Offset line_start_;
  std::unique_ptr<TextScanner> text_scanner_;
  const float zoom_;

  DISALLOW_COPY_AND_ASSIGN(TextFormatter);
};

}  // namespace layout

#endif  // EVITA_LAYOUT_TEXT_FORMATTER_H_
