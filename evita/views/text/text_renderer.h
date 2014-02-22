// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_views_text_text_renderer_h)
#define INCLUDE_evita_views_text_text_renderer_h

#include <memory>

#include "evita/gfx_base.h"
#include "evita/vi_style.h"
#include "evita/views/text/render_selection.h"

class Font;
class Style;

namespace views {

namespace rendering {
  class Cell;
  class ScreenTextBlock;
  class TextBlock;
  class TextFormatter;
  class TextBlock;
  class TextLine;
};

//////////////////////////////////////////////////////////////////////
//
// TextRenderer
//
class TextRenderer {
  private: typedef common::win::Rect Rect;
  private: typedef rendering::Cell Cell;
  public: typedef rendering::ScreenTextBlock ScreenTextBlock;
  private: typedef rendering::Selection Selection;
  public: typedef rendering::TextBlock TextBlock;
  public: typedef rendering::TextLine Line;

  private: const gfx::Graphics* gfx_;
  private: text::Buffer* const m_pBuffer;
  private: Posn m_lStart;
  private: Posn m_lEnd;
  private: Color m_crBackground;
  private: std::unique_ptr<ScreenTextBlock> screen_text_block_;
  private: Selection selection_;
  private: std::unique_ptr<TextBlock> text_block_;

  public: TextRenderer(text::Buffer* buffer);
  public: ~TextRenderer();

  // [F]
  private: void fillBottom() const;
  private: void fillRight(const Line*) const;
  public: Line* FindLine(Posn) const;
  public: void Format(Posn start);
  public: Line* FormatLine(Posn start);

  // [G]
  public: text::Buffer* GetBuffer() const { return m_pBuffer; }
  public: Posn GetStart() const { return m_lStart; }
  public: Posn GetEnd() const { return m_lEnd; }

  // [I]
  private: bool isPosnVisible(Posn) const;

  // [M]
  public: void MakePosnVisible(Posn);
  public: Posn MapPointToPosn(gfx::PointF) const;
  public: gfx::RectF MapPosnToPoint(Posn) const;

  // [P]
  private: int pageLines() const;
  public: void Prepare(const Selection&);

  // [R]
  public: void Render();
  public: void Reset();

  // [S]
  public: bool ScrollDown();
  public: bool ScrollToPosn(Posn target_position);
  public: bool ScrollUp();
  public: void SetGraphics(const gfx::Graphics* gfx);
  public: void SetRect(const Rect& rect);
  public: bool ShouldFormat(const Selection& selection,
                            bool check_selection_color = false) const;
  public: bool ShouldRender() const;

  DISALLOW_COPY_AND_ASSIGN(TextRenderer);
};

}  // namespace views

#endif //!defined(INCLUDE_evita_views_text_text_renderer_h)
