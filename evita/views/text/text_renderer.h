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
class TextRenderer final {
  private: typedef common::win::Rect Rect;
  private: typedef rendering::Cell Cell;
  public: typedef rendering::ScreenTextBlock ScreenTextBlock;
  public: typedef rendering::TextSelectionModel TextSelectionModel;
  public: typedef rendering::TextBlock TextBlock;
  public: typedef rendering::TextLine Line;

  private: gfx::Canvas* canvas_;
  private: text::Buffer* const m_pBuffer;
  private: std::unique_ptr<ScreenTextBlock> screen_text_block_;
  private: bool should_format_;
  private: bool should_render_;
  private: std::unique_ptr<TextBlock> text_block_;
  private: float zoom_;

  public: TextRenderer(text::Buffer* buffer);
  public: ~TextRenderer();

  public: float zoom() const { return zoom_; }
  public: void set_zoom(float new_zoom);

  // [D]
  public: void DidKillFocus();
  public: void DidSetFocus();

  // [F]
  public: Line* FindLine(Posn) const;
  public: void Format(Posn start);
  public: Line* FormatLine(Posn start);

  // [G]
  public: text::Buffer* GetBuffer() const { return m_pBuffer; }
  public: Posn GetStart() const;
  public: Posn GetEnd() const;
  // Returns fully visible end offset or end of line position if there is only
  // one line.
  public: Posn GetVisibleEnd() const;

  // [H]
  public: gfx::RectF HitTestTextPosition(Posn) const;

  // [I]
  public: bool IsPositionFullyVisible(Posn) const;

  // [M]
  public: Posn MapPointToPosition(gfx::PointF point) const;

  // [R]
  public: void Render(const TextSelectionModel& selection);
  public: void RenderSelectionIfNeeded(const TextSelectionModel& selection);
  public: void Reset();

  // [S]
  public: bool ScrollDown();
  public: bool ScrollToPosition(Posn offset);
  public: bool ScrollUp();
  public: void SetBounds(const Rect& rect);
  public: void SetCanvas(gfx::Canvas* canvas);
  public: bool ShouldFormat() const;
  public: bool ShouldRender() const;

  DISALLOW_COPY_AND_ASSIGN(TextRenderer);
};

}  // namespace views

#endif //!defined(INCLUDE_evita_views_text_text_renderer_h)
