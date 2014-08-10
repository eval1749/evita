// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_views_text_text_renderer_h)
#define INCLUDE_evita_views_text_text_renderer_h

#include <memory>

#include "evita/gfx/canvas.h"
#include "evita/gfx/rect.h"
#include "evita/gfx/rect_f.h"
#include "evita/text/buffer_mutation_observer.h"
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
class TextRenderer final : public text::BufferMutationObserver {
  private: typedef rendering::Cell Cell;
  public: typedef rendering::ScreenTextBlock ScreenTextBlock;
  public: typedef rendering::TextSelectionModel TextSelectionModel;
  public: typedef rendering::TextBlock TextBlock;
  public: typedef rendering::TextLine TextLine;

  private: gfx::RectF bounds_;
  private: text::Buffer* const buffer_;
  private: std::unique_ptr<ScreenTextBlock> screen_text_block_;
  private: bool should_format_;
  private: bool should_render_;
  private: std::unique_ptr<TextBlock> text_block_;
  private: text::Posn view_start_;
  private: float zoom_;

  public: TextRenderer(text::Buffer* buffer);
  public: ~TextRenderer();

  public: text::Buffer* buffer() const { return buffer_; }
  public: float zoom() const { return zoom_; }
  public: void set_zoom(float new_zoom);

  public: void DidHide();
  public: void DidKillFocus(gfx::Canvas* canvas);
  public: void DidLostCanvas();
  public: void DidSetFocus();
  // Returns end of line offset containing |text_offset|.
  public: text::Posn EndOfLine(text::Posn text_offset) const;
  private: TextLine* FindLine(text::Posn text_offset) const;
  public: void Format(text::Posn text_offset);
  // Returns true if formatting taken place.
  public: bool FormatIfNeeded();
  public: Posn GetStart();
  public: Posn GetEnd();
  // Returns fully visible end offset or end of line position if there is only
  // one line.
  public: Posn GetVisibleEnd();
  public: gfx::RectF HitTestTextPosition(text::Posn text_offset) const;
  public: bool IsPositionFullyVisible(Posn) const;
  public: text::Posn MapPointToPosition(gfx::PointF point);
  public: text::Posn MapPointXToOffset(text::Posn text_offset,
                                       float point_x) const;
  public: void Render(gfx::Canvas* canvas, const TextSelectionModel& selection);
  private: void RenderRuler(gfx::Canvas* canvas);
  public: void RenderSelectionIfNeeded(gfx::Canvas* canvas,
                                       const TextSelectionModel& selection);
  public: bool ScrollDown();
  public: bool ScrollToPosition(Posn offset);
  public: bool ScrollUp();
  public: void SetBounds(const gfx::RectF& new_bounds);
  private: bool ShouldFormat() const;
  public: bool ShouldRender() const;
  // Returns start of line offset containing |text_offset|.
  public: text::Posn StartOfLine(text::Posn text_offset) const;

  // text::BufferMutationObserver
  private: virtual void DidDeleteAt(text::Posn offset, size_t length) override;
  private: virtual void DidInsertAt(text::Posn offset, size_t length) override;

  DISALLOW_COPY_AND_ASSIGN(TextRenderer);
};

}  // namespace views

#endif //!defined(INCLUDE_evita_views_text_text_renderer_h)
