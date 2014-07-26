// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_views_text_text_renderer_h)
#define INCLUDE_evita_views_text_text_renderer_h

#include <memory>

#include "evita/gfx/canvas.h"
#include "evita/gfx/rect.h"
#include "evita/gfx/rect_f.h"
#include "evita/views/text/render_selection.h"

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
  private: typedef rendering::Cell Cell;
  public: typedef rendering::ScreenTextBlock ScreenTextBlock;
  public: typedef rendering::TextSelectionModel TextSelectionModel;
  public: typedef rendering::TextBlock TextBlock;
  public: typedef rendering::TextLine Line;

  private: gfx::RectF bounds_;
  private: text::Buffer* const buffer_;
  private: std::unique_ptr<ScreenTextBlock> screen_text_block_;
  private: bool should_format_;
  private: bool should_render_;
  private: std::unique_ptr<TextBlock> text_block_;
  private: float zoom_;

  public: TextRenderer(text::Buffer* buffer);
  public: ~TextRenderer();

  public: text::Buffer* buffer() const { return buffer_; }
  public: float zoom() const { return zoom_; }
  public: void set_zoom(float new_zoom);

  // [D]
  public: void DidHide();
  public: void DidKillFocus(gfx::Canvas* canvas);
  public: void DidLostCanvas();
  public: void DidSetFocus();

  // [F]
  public: Line* FindLine(Posn) const;
  public: void Format(Posn start);
  public: Line* FormatLine(Posn start);

  // [G]
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
  public: void Render(gfx::Canvas* canvas, const TextSelectionModel& selection);
  private: void RenderRuler(gfx::Canvas* canvas);
  public: void RenderSelectionIfNeeded(gfx::Canvas* canvas,
                                       const TextSelectionModel& selection);

  // [S]
  public: bool ScrollDown();
  public: bool ScrollToPosition(Posn offset);
  public: bool ScrollUp();
  public: void SetBounds(const gfx::RectF& new_bounds);
  public: bool ShouldFormat() const;
  public: bool ShouldRender() const;

  DISALLOW_COPY_AND_ASSIGN(TextRenderer);
};

}  // namespace views

#endif //!defined(INCLUDE_evita_views_text_text_renderer_h)
