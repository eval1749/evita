// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_views_text_text_formatter_h)
#define INCLUDE_evita_views_text_text_formatter_h

#include <memory>

#include "base/basictypes.h"
#include "evita/text/style.h"
#include "evita/gfx_base.h"

namespace text {
class Buffer;
}

namespace views {
namespace rendering {

class Cell;
struct Selection;
class TextBlock;
class TextLine;
enum class TextMarker;

class TextFormatter {
  private: class EnumCI;

  private: text::Color const filler_color_;
  private: const gfx::Graphics& m_gfx;
  private: const Selection& selection_;
  private: TextBlock* const text_block_;
  private: std::unique_ptr<EnumCI> m_oEnumCI;

  public: TextFormatter(const gfx::Graphics& gfx, TextBlock* text_block,
                        text::Buffer* buffer, Posn start,
                        const Selection& selection);
  public: ~TextFormatter();

  public: void Format();
  public: TextLine* FormatLine();

  private: Cell* formatChar(Cell*, float x, char16);
  private: Cell* formatMarker(TextMarker marker_name);

  DISALLOW_COPY_AND_ASSIGN(TextFormatter);
};

} // namespace rendering
} // namespace views

#endif //!defined(INCLUDE_evita_views_text_text_formatter_h)
