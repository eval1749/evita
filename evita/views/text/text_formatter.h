// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_views_text_text_formatter_h)
#define INCLUDE_evita_views_text_text_formatter_h

#include <memory>

#include "base/basictypes.h"
#include "evita/gfx_base.h"
#include "evita/views/text/render_style.h"

namespace text {
class Buffer;
}

namespace views {
namespace rendering {

class Cell;
class TextBlock;
class TextLine;
enum class TextMarker;
class TextSelection;
class TextSelectionModel;

class TextFormatter final {
  private: class TextScanner;

  private: RenderStyle default_render_style_;
  private: css::Style default_style_;
  private: TextBlock* const text_block_;
  private: std::unique_ptr<TextScanner> text_scanner_;
  private: float zoom_;

  public: TextFormatter(TextBlock* text_block, text::Posn start);
  public: ~TextFormatter();

  public: void Format();
  public: TextLine* FormatLine();

  private: Cell* formatChar(Cell*, float x, char16);
  private: Cell* formatMarker(TextMarker marker_name);

  public: static TextSelection FormatSelection(
      const text::Buffer* buffer, const TextSelectionModel& selection_model);

  DISALLOW_COPY_AND_ASSIGN(TextFormatter);
};

} // namespace rendering
} // namespace views

#endif //!defined(INCLUDE_evita_views_text_text_formatter_h)
