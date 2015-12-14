// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_LAYOUT_TEXT_FORMATTER_H_
#define EVITA_LAYOUT_TEXT_FORMATTER_H_

#include <memory>

#include "base/basictypes.h"
#include "evita/gfx_base.h"
#include "evita/precomp.h"
#include "evita/layout/render_style.h"

namespace text {
class Buffer;
}

namespace layout {

class InlineBox;
class RootInlineBox;
enum class TextMarker;
class TextSelection;
class TextSelectionModel;

class TextFormatter final {
 public:
  TextFormatter(const text::Buffer* buffer,
                text::Posn text_offset,
                const gfx::RectF& bounds,
                float zoom);
  ~TextFormatter();

  text::Posn text_offset() const;
  void set_text_offset(text::Posn new_text_offset);

  RootInlineBox* FormatLine(text::Posn text_offset);
  RootInlineBox* FormatLine();
  InlineBox* FormatMarker(TextMarker marker_name);

  static TextSelection FormatSelection(
      const text::Buffer* buffer,
      const TextSelectionModel& selection_model);

 private:
  class TextScanner;

  InlineBox* FormatChar(InlineBox*, float x, base::char16);

  gfx::RectF bounds_;
  RenderStyle default_render_style_;
  std::unique_ptr<TextScanner> text_scanner_;
  float zoom_;

  DISALLOW_COPY_AND_ASSIGN(TextFormatter);
};

}  // namespace layout

#endif  // EVITA_LAYOUT_TEXT_FORMATTER_H_
