// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_views_text_render_text_formatter_h)
#define INCLUDE_evita_views_text_render_text_formatter_h

#include <memory>

#include "base/basictypes.h"
#include "evita/gfx_base.h"

namespace views {
class TextRenderer;

namespace rendering {

class Cell;
class TextLine;
enum class TextMarker;

class TextFormatter {
  private: class EnumCI;

  private: const gfx::Graphics& m_gfx;
  private: TextRenderer* const m_pTextRenderer;
  private: std::unique_ptr<EnumCI> m_oEnumCI;

  // TODO(yosi) We should use |TextBlock| instead of |TextRenderer| in
  // |TextFormatter|.
  public: TextFormatter(const gfx::Graphics& gfx, TextRenderer* renderer,
                        Posn lStart);
  public: ~TextFormatter();

  public: void Format();
  public: bool FormatLine(TextLine* line);

  private: Cell* formatChar(Cell*, float x, char16);
  private: Cell* formatMarker(TextMarker marker_name);

  DISALLOW_COPY_AND_ASSIGN(TextFormatter);
};

} // namespace rendering
} // namespace views

#endif //!defined(INCLUDE_evita_views_text_render_text_formatter_h)
